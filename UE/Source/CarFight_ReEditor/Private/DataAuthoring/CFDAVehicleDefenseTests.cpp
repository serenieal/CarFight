// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseTests.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-01 VehicleDefenseData provider/strict contract/recursive Reflection/protected current focused Automation입니다.
// Changelog:
// - v1.0.0: exact17+nested12 parse/serialize/fingerprint, provider readiness, negative contract, predecessor recursive Reflection parity와 protected DA read-only current resolver를 검증합니다.
// Migration:
// - 실제 Save/Delete는 수행하지 않습니다. persisted DA_VehicleDefense_Test는 read-only로만 관측합니다.
// - VehicleDefense DACE bootstrap/history와 mixed operational admission은 VDR-P0-02 전까지 검증·변경하지 않습니다.

#include "CFDAVehicleDefenseProvider.h"
#include "CFDAAmmoDace.h"
#include "CFDACommonPrimitives.h"
#include "CFDAContractGuard.h"
#include "CFDADamageDace.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStaging.h"

#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFMissileGuidePresetData.h"
#include "CFVehicleDefenseData.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAVehicleDefenseTestsPrivate
{
	// VDR-P0-01 memory-only synthetic target path입니다.
	static const FString SyntheticTargetPath = TEXT("/Game/Test/CarFight/VDR/DA_VehicleDefense_ReadOnly.DA_VehicleDefense_ReadOnly");

	// VDR-P0-01 memory-only synthetic stable identity입니다.
	static const FName SyntheticDefenseId(TEXT("Defense_VDR_P0_01_ReadOnly"));

	// Gate 1에서 read-only protected baseline으로 동결한 persisted VehicleDefenseData path입니다.
	static const FString ProtectedDefensePath = TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test.DA_VehicleDefense_Test");

	// 한 directional armor config를 deterministic valid 값으로 만듭니다.
	FCFDirectionalArmorConfig BuildArmorConfig(const float MaximumArmor, const float DamageMultiplier)
	{
		// 반환할 authored directional armor config입니다.
		FCFDirectionalArmorConfig Config;
		Config.MaximumArmor = MaximumArmor;
		Config.DamageMultiplier = DamageMultiplier;
		return Config;
	}

	// Gate 1 exact23 semantic leaf를 모두 채운 valid VehicleDefense payload를 만듭니다.
	FCFDAVehicleDefensePayload BuildValidPayload()
	{
		// 반환할 valid VehicleDefense payload입니다.
		FCFDAVehicleDefensePayload Payload;
		Payload.DefenseId = SyntheticDefenseId;
		Payload.DefenseMassKg = 125.0f;
		Payload.bUseShield = true;
		Payload.MaximumShield = 500.0f;
		Payload.ShieldRegenerationDelaySeconds = 3.0f;
		Payload.ShieldRegenerationPerSecond = 25.0f;
		Payload.ArmorType = ECFArmorType::Heavy;
		Payload.ArmorResistance = 80.0f;
		Payload.FrontArmorConfig = BuildArmorConfig(300.0f, 0.75f);
		Payload.LeftArmorConfig = BuildArmorConfig(220.0f, 0.90f);
		Payload.RightArmorConfig = BuildArmorConfig(220.0f, 0.90f);
		Payload.RearArmorConfig = BuildArmorConfig(160.0f, 1.10f);
		Payload.TopArmorConfig = BuildArmorConfig(100.0f, 1.20f);
		Payload.BottomArmorConfig = BuildArmorConfig(120.0f, 1.15f);
		Payload.ShieldComponentDamageScale = 0.20f;
		Payload.ArmorComponentDamageScale = 0.35f;
		Payload.IntegrityComponentDamageScale = 0.50f;
		return Payload;
	}

	// Valid payload를 production serializer로 whole-record JSON으로 만듭니다.
	bool BuildValidJson(FString& OutJson, FString& OutError)
	{
		return CFDAVehicleDefenseProviderImpl::SerializeStagingJson(
			BuildValidPayload(),
			SyntheticTargetPath,
			FString(),
			OutJson,
			OutError);
	}

	// JSON text를 mutable root object로 parse합니다.
	bool ParseJsonObject(const FString& JsonText, TSharedPtr<FJsonObject>& OutObject)
	{
		// JSON source reader입니다.
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}

	// Mutable root object를 JSON text로 직렬화합니다.
	bool WriteJsonObject(const TSharedPtr<FJsonObject>& Object, FString& OutJson)
	{
		OutJson.Reset();
		// JSON destination writer입니다.
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(Object.ToSharedRef(), Writer))
		{
			return false;
		}
		Writer->Close();
		return true;
	}

	// Expected/observed SourceShape canonical row coverage가 exact인지 검사합니다.
	bool HasExactSourceCoverage(
		const TArray<FCFDASourceFieldDescriptor>& Expected,
		const TArray<FCFDASourceFieldDescriptor>& Observed)
	{
		return FCFDAContractGuard::ValidateSourceShapeCoverage(Expected, Observed).bPassed;
	}

	// SourceShape에 exact property path가 존재하는지 확인합니다.
	bool HasSourcePath(const TArray<FCFDASourceFieldDescriptor>& Descriptors, const FString& PropertyPath)
	{
		for (const FCFDASourceFieldDescriptor& Descriptor : Descriptors)
		{
			if (Descriptor.SourcePropertyPath.Equals(PropertyPath, ESearchCase::CaseSensitive))
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefProviderTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_01.ProviderContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefStrictTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_01.StrictContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefReflectTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_01.ReflectionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefCurrentTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_01.ProtectedCurrentReadOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// VehicleDefense exact provider registry/readiness와 parse/serialize/fingerprint/raw preservation을 검증합니다.
bool FCFDAVDefProviderTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseTestsPrivate;

	// Production exact4 provider registry validation 상세입니다.
	FString RegistryError;
	TestTrue(TEXT("Production provider registry exact4 must validate"), CFDATypeDispatch::ValidateProviderRegistry(RegistryError));

	// VehicleDefense exact TypeKey lookup 상세입니다.
	FString LookupError;
	// VehicleDefense production provider entry입니다.
	const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		TEXT("CarFight.DataAsset.VehicleDefenseData"),
		TEXT("/Script/CarFight_Re.CFVehicleDefenseData"),
		&LookupError);
	TestTrue(TEXT("VehicleDefense exact TypeKey must resolve production provider"), ProviderEntry == &CFDAVehicleDefenseProvider::GetProvider());
	if (ProviderEntry == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("VehicleDefense readiness"), ProviderEntry->Readiness, ECFDAProviderReadiness::ReviewedMutationReady);
	TestEqual(TEXT("VehicleDefense DACE remains not ready in Gate2"), ProviderEntry->Descriptor.DaceReadiness, ECFDADaceReadiness::ContractNotReady);
	TestTrue(TEXT("VehicleDefense canonical target set is explicitly declared"), ProviderEntry->Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("VehicleDefense canonical target set remains exact0"), ProviderEntry->Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);
	TestTrue(TEXT("VehicleDefense parse callback exists"), ProviderEntry->Operations.ParseCommonCandidate != nullptr);
	TestTrue(TEXT("VehicleDefense current callback exists"), ProviderEntry->Operations.ResolveCommonCurrentState != nullptr);
	TestTrue(TEXT("VehicleDefense reviewed apply callback exists"), ProviderEntry->Operations.ApplyReviewedMutation != nullptr);

	// Reviewed mutation readiness validation detail입니다.
	FString MutationReadyError;
	TestTrue(TEXT("VehicleDefense provider is mutation-ready locally"), CFDATypeDispatch::ValidateProviderMutationReady(*ProviderEntry, MutationReadyError));

	// Canonical valid whole-record JSON입니다.
	FString ValidJson;
	// Serializer failure detail입니다.
	FString SerializeError;
	TestTrue(TEXT("VehicleDefense valid payload serializes"), BuildValidJson(ValidJson, SerializeError));
	// Production strict parse 결과입니다.
	const FCFDAVehicleDefenseParseResult Parsed = CFDAVehicleDefenseProviderImpl::ParseJson(
		ValidJson,
		TEXT("Authoring/DataAssetStaging/VehicleDefenseData/__AutomationP01__/Provider.json"));
	TestTrue(TEXT("VehicleDefense valid exact17+nested12 JSON parses"), Parsed.bValid);
	TestTrue(TEXT("VehicleDefense staging fingerprint is canonical sha256"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Parsed.Record.StagingSemanticFingerprint));
	TestEqual(TEXT("VehicleDefense parsed ArmorType"), Parsed.Record.Payload.ArmorType, ECFArmorType::Heavy);
	TestEqual(TEXT("VehicleDefense parsed front MaximumArmor"), Parsed.Record.Payload.FrontArmorConfig.MaximumArmor, 300.0f);
	TestEqual(TEXT("VehicleDefense parsed bottom DamageMultiplier"), Parsed.Record.Payload.BottomArmorConfig.DamageMultiplier, 1.15f);

	// Nested semantic 한 값만 바꾼 payload입니다.
	FCFDAVehicleDefensePayload NestedChangedPayload = BuildValidPayload();
	NestedChangedPayload.FrontArmorConfig.DamageMultiplier = 0.76f;
	// Nested semantic 변경 fingerprint입니다.
	FString NestedChangedFingerprint;
	// Nested semantic fingerprint 실패 상세입니다.
	FString FingerprintError;
	TestTrue(TEXT("Nested changed VehicleDefense payload fingerprints"), CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(NestedChangedPayload, NestedChangedFingerprint, FingerprintError));
	TestNotEqual(TEXT("Nested armor semantic change alters fingerprint"), Parsed.Record.StagingSemanticFingerprint, NestedChangedFingerprint);

	// bUseShield=false에서도 authored raw shield 값을 보존해야 하는 payload입니다.
	FCFDAVehicleDefensePayload DisabledShieldPayload = BuildValidPayload();
	DisabledShieldPayload.bUseShield = false;
	DisabledShieldPayload.MaximumShield = 777.0f;
	DisabledShieldPayload.ShieldRegenerationDelaySeconds = 9.0f;
	DisabledShieldPayload.ShieldRegenerationPerSecond = 33.0f;
	// Disabled shield whole-record JSON입니다.
	FString DisabledShieldJson;
	TestTrue(
		TEXT("Disabled shield raw-value payload serializes"),
		CFDAVehicleDefenseProviderImpl::SerializeStagingJson(DisabledShieldPayload, SyntheticTargetPath, FString(), DisabledShieldJson, SerializeError));
	// Disabled shield strict parse 결과입니다.
	const FCFDAVehicleDefenseParseResult DisabledShieldParsed = CFDAVehicleDefenseProviderImpl::ParseJson(
		DisabledShieldJson,
		TEXT("Authoring/DataAssetStaging/VehicleDefenseData/__AutomationP01__/Disabled.json"));
	TestTrue(TEXT("Disabled shield record remains valid"), DisabledShieldParsed.bValid);
	TestFalse(TEXT("Disabled shield flag preserved"), DisabledShieldParsed.Record.Payload.bUseShield);
	TestEqual(TEXT("Disabled MaximumShield raw value preserved"), DisabledShieldParsed.Record.Payload.MaximumShield, 777.0f);
	TestEqual(TEXT("Disabled regen delay raw value preserved"), DisabledShieldParsed.Record.Payload.ShieldRegenerationDelaySeconds, 9.0f);
	TestEqual(TEXT("Disabled regen rate raw value preserved"), DisabledShieldParsed.Record.Payload.ShieldRegenerationPerSecond, 33.0f);
	return !HasAnyErrors();
}

// VehicleDefense unknown/missing/nested/enum/identity/finite/range/cross-field contract를 fail-closed 검증합니다.
bool FCFDAVDefStrictTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseTestsPrivate;

	// Mutation에 사용할 valid root JSON입니다.
	FString ValidJson;
	// Valid serializer failure detail입니다.
	FString SerializeError;
	if (!TestTrue(TEXT("Strict test valid baseline serializes"), BuildValidJson(ValidJson, SerializeError)))
	{
		AddError(SerializeError);
		return false;
	}

	// Unknown top-level field fixture root입니다.
	TSharedPtr<FJsonObject> UnknownRoot;
	TestTrue(TEXT("Parse baseline root for unknown field"), ParseJsonObject(ValidJson, UnknownRoot));
	UnknownRoot->SetNumberField(TEXT("Unexpected"), 1.0);
	// Unknown-field JSON입니다.
	FString UnknownJson;
	TestTrue(TEXT("Write unknown-field JSON"), WriteJsonObject(UnknownRoot, UnknownJson));
	// Unknown-field parse result입니다.
	const FCFDAVehicleDefenseParseResult UnknownResult = CFDAVehicleDefenseProviderImpl::ParseJson(UnknownJson);
	TestFalse(TEXT("Unknown root field rejected"), UnknownResult.bValid);
	TestTrue(TEXT("Unknown root diagnostic"), FCFDAStagingService::HasIssueCode(UnknownResult.Issues, ECFDAStagingIssueCode::UnknownField));

	// Missing nested child fixture root입니다.
	TSharedPtr<FJsonObject> MissingNestedRoot;
	TestTrue(TEXT("Parse baseline root for missing nested field"), ParseJsonObject(ValidJson, MissingNestedRoot));
	// Missing nested fixture payload입니다.
	const TSharedPtr<FJsonObject> MissingNestedPayload = MissingNestedRoot->GetObjectField(TEXT("Payload"));
	// Missing nested fixture front armor object입니다.
	const TSharedPtr<FJsonObject> MissingNestedFront = MissingNestedPayload->GetObjectField(TEXT("FrontArmorConfig"));
	MissingNestedFront->RemoveField(TEXT("MaximumArmor"));
	// Missing nested child JSON입니다.
	FString MissingNestedJson;
	TestTrue(TEXT("Write missing nested JSON"), WriteJsonObject(MissingNestedRoot, MissingNestedJson));
	// Missing nested parse result입니다.
	const FCFDAVehicleDefenseParseResult MissingNestedResult = CFDAVehicleDefenseProviderImpl::ParseJson(MissingNestedJson);
	TestFalse(TEXT("Missing nested armor child rejected"), MissingNestedResult.bValid);
	TestTrue(TEXT("Missing nested child diagnostic"), FCFDAStagingService::HasIssueCode(MissingNestedResult.Issues, ECFDAStagingIssueCode::MissingRequiredField));

	// Wrong-case ArmorType fixture root입니다.
	TSharedPtr<FJsonObject> WrongEnumRoot;
	TestTrue(TEXT("Parse baseline root for enum token"), ParseJsonObject(ValidJson, WrongEnumRoot));
	// Wrong-case enum fixture payload입니다.
	const TSharedPtr<FJsonObject> WrongEnumPayload = WrongEnumRoot->GetObjectField(TEXT("Payload"));
	WrongEnumPayload->SetStringField(TEXT("ArmorType"), TEXT("heavy"));
	// Wrong enum token JSON입니다.
	FString WrongEnumJson;
	TestTrue(TEXT("Write wrong enum JSON"), WriteJsonObject(WrongEnumRoot, WrongEnumJson));
	// Wrong enum token parse result입니다.
	const FCFDAVehicleDefenseParseResult WrongEnumResult = CFDAVehicleDefenseProviderImpl::ParseJson(WrongEnumJson);
	TestFalse(TEXT("Wrong-case ArmorType rejected"), WrongEnumResult.bValid);
	TestTrue(TEXT("ArmorType enum diagnostic"), FCFDAStagingService::HasIssueCode(WrongEnumResult.Issues, ECFDAStagingIssueCode::InvalidEnumValue));

	// Split identity fixture root입니다.
	TSharedPtr<FJsonObject> IdentityRoot;
	TestTrue(TEXT("Parse baseline root for identity split"), ParseJsonObject(ValidJson, IdentityRoot));
	IdentityRoot->SetStringField(TEXT("StableLogicalId"), TEXT("Different_Defense_Id"));
	// Split identity JSON입니다.
	FString IdentityJson;
	TestTrue(TEXT("Write split identity JSON"), WriteJsonObject(IdentityRoot, IdentityJson));
	// Split identity parse result입니다.
	const FCFDAVehicleDefenseParseResult IdentityResult = CFDAVehicleDefenseProviderImpl::ParseJson(IdentityJson);
	TestFalse(TEXT("StableLogicalId/DefenseId split rejected"), IdentityResult.bValid);
	TestTrue(TEXT("Identity split diagnostic"), FCFDAStagingService::HasIssueCode(IdentityResult.Issues, ECFDAStagingIssueCode::StableIdentityMismatch));

	// Negative nested numeric fixture root입니다.
	TSharedPtr<FJsonObject> NegativeNestedRoot;
	TestTrue(TEXT("Parse baseline root for negative nested value"), ParseJsonObject(ValidJson, NegativeNestedRoot));
	// Negative nested fixture payload입니다.
	const TSharedPtr<FJsonObject> NegativeNestedPayload = NegativeNestedRoot->GetObjectField(TEXT("Payload"));
	// Negative nested fixture rear armor object입니다.
	const TSharedPtr<FJsonObject> NegativeRear = NegativeNestedPayload->GetObjectField(TEXT("RearArmorConfig"));
	NegativeRear->SetNumberField(TEXT("DamageMultiplier"), -0.1);
	// Negative nested JSON입니다.
	FString NegativeNestedJson;
	TestTrue(TEXT("Write negative nested JSON"), WriteJsonObject(NegativeNestedRoot, NegativeNestedJson));
	// Negative nested parse result입니다.
	const FCFDAVehicleDefenseParseResult NegativeNestedResult = CFDAVehicleDefenseProviderImpl::ParseJson(NegativeNestedJson);
	TestFalse(TEXT("Negative nested DamageMultiplier rejected"), NegativeNestedResult.bValid);
	TestTrue(TEXT("Negative nested diagnostic"), FCFDAStagingService::HasIssueCode(NegativeNestedResult.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Invalid shield cross-field fixture root입니다.
	TSharedPtr<FJsonObject> InvalidShieldRoot;
	TestTrue(TEXT("Parse baseline root for shield invariant"), ParseJsonObject(ValidJson, InvalidShieldRoot));
	// Invalid shield fixture payload입니다.
	const TSharedPtr<FJsonObject> InvalidShieldPayload = InvalidShieldRoot->GetObjectField(TEXT("Payload"));
	InvalidShieldPayload->SetBoolField(TEXT("bUseShield"), true);
	InvalidShieldPayload->SetNumberField(TEXT("MaximumShield"), 0.0);
	InvalidShieldPayload->SetNumberField(TEXT("ShieldRegenerationPerSecond"), 1.0);
	// Invalid shield JSON입니다.
	FString InvalidShieldJson;
	TestTrue(TEXT("Write invalid shield JSON"), WriteJsonObject(InvalidShieldRoot, InvalidShieldJson));
	// Invalid shield parse result입니다.
	const FCFDAVehicleDefenseParseResult InvalidShieldResult = CFDAVehicleDefenseProviderImpl::ParseJson(InvalidShieldJson);
	TestFalse(TEXT("Enabled zero-capacity regenerating shield rejected"), InvalidShieldResult.bValid);
	TestTrue(TEXT("Shield invariant diagnostic"), FCFDAStagingService::HasIssueCode(InvalidShieldResult.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Non-finite typed payload fixture입니다.
	FCFDAVehicleDefensePayload NonFinitePayload = BuildValidPayload();
	NonFinitePayload.TopArmorConfig.MaximumArmor = std::numeric_limits<float>::infinity();
	// Non-finite fingerprint output입니다.
	FString NonFiniteFingerprint;
	// Non-finite validation detail입니다.
	FString NonFiniteError;
	TestFalse(TEXT("Non-finite nested typed value cannot fingerprint"), CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(NonFinitePayload, NonFiniteFingerprint, NonFiniteError));
	return !HasAnyErrors();
}

// Generic recursive Reflection seam이 VehicleDefense exact29를 관측하고 predecessor accepted SourceShape를 바꾸지 않는지 검증합니다.
bool FCFDAVDefReflectTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseTestsPrivate;

	// VehicleDefense direct-only Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> VehicleDefenseDirect;
	// VehicleDefense direct Reflection failure detail입니다.
	FString ReflectionError;
	TestTrue(TEXT("VehicleDefense direct Reflection succeeds"), FCFDAContractGuard::BuildDirectReflectedSourceShapeDescriptor(*UCFVehicleDefenseData::StaticClass(), VehicleDefenseDirect, ReflectionError));
	TestEqual(TEXT("VehicleDefense direct top-level authored nodes exact17"), VehicleDefenseDirect.Num(), 17);

	// VehicleDefense recursive Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> VehicleDefenseRecursive;
	TestTrue(TEXT("VehicleDefense recursive Reflection succeeds"), FCFDAContractGuard::BuildRecursiveReflectedSourceShapeDescriptor(*UCFVehicleDefenseData::StaticClass(), VehicleDefenseRecursive, ReflectionError));
	TestEqual(TEXT("VehicleDefense recursive SourceShape exact29"), VehicleDefenseRecursive.Num(), 29);
	TestTrue(TEXT("Front nested MaximumArmor observed"), HasSourcePath(VehicleDefenseRecursive, TEXT("FrontArmorConfig.MaximumArmor")));
	TestTrue(TEXT("Bottom nested DamageMultiplier observed"), HasSourcePath(VehicleDefenseRecursive, TEXT("BottomArmorConfig.DamageMultiplier")));

	// Ammo generic recursive Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> AmmoRecursive;
	TestTrue(TEXT("Ammo recursive Reflection succeeds"), FCFDAContractGuard::BuildRecursiveReflectedSourceShapeDescriptor(*UCFAmmoData::StaticClass(), AmmoRecursive, ReflectionError));
	TestEqual(TEXT("Ammo recursive remains exact8"), AmmoRecursive.Num(), 8);
	TestTrue(TEXT("Ammo recursive matches accepted SourceShape"), HasExactSourceCoverage(FCFDAAmmoDace::GetSourceShapeDescriptor(), AmmoRecursive));

	// Damage generic recursive Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> DamageRecursive;
	TestTrue(TEXT("Damage recursive Reflection succeeds"), FCFDAContractGuard::BuildRecursiveReflectedSourceShapeDescriptor(*UCFDamageData::StaticClass(), DamageRecursive, ReflectionError));
	TestEqual(TEXT("Damage recursive remains exact12"), DamageRecursive.Num(), 12);
	TestTrue(TEXT("Damage recursive matches accepted SourceShape"), HasExactSourceCoverage(FCFDADamageDace::GetSourceShapeDescriptor(), DamageRecursive));

	// Missile generic recursive Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> MissileRecursive;
	TestTrue(TEXT("Missile recursive Reflection succeeds"), FCFDAContractGuard::BuildRecursiveReflectedSourceShapeDescriptor(*UCFMissileGuidePresetData::StaticClass(), MissileRecursive, ReflectionError));
	TestEqual(TEXT("Missile recursive remains exact30"), MissileRecursive.Num(), 30);
	TestTrue(TEXT("Missile recursive matches accepted compatibility SourceShape"), HasExactSourceCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), MissileRecursive));
	return !HasAnyErrors();
}

// Protected DA_VehicleDefense_Test를 mutation 없이 raw extractor/fingerprint/current resolver로 읽습니다.
bool FCFDAVDefCurrentTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseTestsPrivate;

	// Read-only protected VehicleDefenseData입니다.
	UCFVehicleDefenseData* ProtectedDefense = LoadObject<UCFVehicleDefenseData>(nullptr, *ProtectedDefensePath);
	TestNotNull(TEXT("Protected DA_VehicleDefense_Test loads"), ProtectedDefense);
	if (ProtectedDefense == nullptr)
	{
		return false;
	}

	// Protected owning package입니다.
	UPackage* ProtectedPackage = ProtectedDefense->GetOutermost();
	TestNotNull(TEXT("Protected VehicleDefense has owning package"), ProtectedPackage);
	if (ProtectedPackage == nullptr)
	{
		return false;
	}
	// 이 test가 변경하면 안 되는 pre-existing package dirty state입니다.
	const bool bPackageDirtyBefore = ProtectedPackage->IsDirty();

	// Protected authored raw whole-record payload입니다.
	FCFDAVehicleDefensePayload ProtectedPayload;
	// Protected extractor diagnostics입니다.
	TArray<FCFDAStagingIssue> ExtractIssues;
	TestTrue(TEXT("Protected VehicleDefense raw extractor succeeds"), CFDAVehicleDefenseProviderImpl::ExtractPayload(*ProtectedDefense, ProtectedPayload, ExtractIssues));
	TestFalse(TEXT("Protected DefenseId remains non-None"), ProtectedPayload.DefenseId.IsNone());

	// Protected current semantic fingerprint입니다.
	FString ProtectedFingerprint;
	// Protected fingerprint failure detail입니다.
	FString FingerprintError;
	TestTrue(TEXT("Protected VehicleDefense fingerprints"), CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(ProtectedPayload, ProtectedFingerprint, FingerprintError));
	TestTrue(TEXT("Protected VehicleDefense fingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ProtectedFingerprint));

	// Current resolver용 provider-local read-only record입니다.
	FCFDAVehicleDefenseRecord CurrentRecord;
	CurrentRecord.SchemaId = CFDAVehicleDefenseProvider::GetProvider().Descriptor.TypeKey.SchemaId;
	CurrentRecord.SchemaRevision = CFDAVehicleDefenseProvider::GetProvider().Descriptor.SchemaRevision;
	CurrentRecord.AdapterContractRevision = CFDAVehicleDefenseProvider::GetProvider().Descriptor.AdapterContractRevision;
	CurrentRecord.DataAssetTypeClassPath = CFDAVehicleDefenseProvider::GetProvider().Descriptor.TypeKey.DataAssetTypeClassPath;
	CurrentRecord.StableLogicalId = ProtectedPayload.DefenseId;
	CurrentRecord.TargetObjectPath = ProtectedDefensePath;
	CurrentRecord.Payload = ProtectedPayload;
	CurrentRecord.StagingSemanticFingerprint = ProtectedFingerprint;
	CurrentRecord.StagingRelativePath = TEXT("Authoring/DataAssetStaging/VehicleDefenseData/__ProtectedReadOnly__/DA_VehicleDefense_Test.json");

	// Protected current payload-free envelope입니다.
	const FCFDACommonEnvelope CurrentEnvelope = CFDAVehicleDefenseProviderImpl::BuildCommonEnvelope(CurrentRecord);
	// Protected current-state readback입니다.
	FCFDACommonCurrentState CurrentState;
	// Current resolver diagnostics입니다.
	TArray<FCFDAStagingIssue> CurrentIssues;
	TestTrue(TEXT("Protected VehicleDefense current resolver succeeds"), CFDAVehicleDefenseProviderImpl::ResolveCommonCurrentState(CurrentEnvelope, CurrentState, CurrentIssues));
	TestTrue(TEXT("Protected VehicleDefense target exists"), CurrentState.bRequestedTargetExists);
	TestEqual(TEXT("Protected VehicleDefense class path"), CurrentState.RequestedTargetClassPath, FString(TEXT("/Script/CarFight_Re.CFVehicleDefenseData")));
	TestEqual(TEXT("Protected VehicleDefense stable identity"), CurrentState.RequestedTargetStableLogicalId, ProtectedPayload.DefenseId);
	TestEqual(TEXT("Protected VehicleDefense current fingerprint"), CurrentState.CurrentSemanticFingerprint, ProtectedFingerprint);
	TestEqual(TEXT("Protected package dirty state unchanged"), ProtectedPackage->IsDirty(), bPackageDirtyBefore);
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
