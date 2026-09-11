// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageProviderTests.cpp
// Version: v1.3.0
// Date: 2026-09-11
// Description: CF-FQ-052 Damage exact12 parse/fingerprint/Preview/provider-boundary focused Automation입니다.
// Changelog:
// - v1.3.0: DDO-P0-04 activation 뒤 stale한 test-local Missile+Ammo exact2 admission literal/assertion을 제거하고 production operational authority 검증은 DDO-P0-04 OperationalAdmission direct-backing test로 이관했습니다.
// - v1.2.0: Damage provider DACE ContractReady activation을 검증하되 ReviewedMutationReady, canonical Product Damage exact0와 mixed operational admission exact2 보존을 고정했습니다.
// - v1.1.0: Damage provider ReviewedMutationReady + non-null Apply callback을 검증하되 DACE ContractNotReady와 mixed operational admission exact2 보존을 고정했습니다.
// - v1.0.1: Mid-review P1 회귀로 BaseDamage exact-zero reject와 radial-enabled radius/damage positive requirement negative cases를 추가.
// - v1.0.0: exact12 strict parse/serialize, semantic fingerprint, ReadOnlyPreviewReady/ContractNotReady, mixed admission exact2, persisted protected exact2 read-only regression을 추가.
// Migration:
// - 이 파일은 memory JSON/DTO와 persisted protected CFDamageData read-only load만 사용합니다. 실제 Save/Delete disposable mutation은 CFDADamageApplyTests.cpp가 소유합니다.
// - bUseRadialDamage=false인 기존 persisted Damage exact2의 zero radial 값은 계속 valid read-only baseline으로 보존합니다. DACE mutation과 Product Damage save는 수행하지 않습니다.

#include "CFDADamageProvider.h"
#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStaging.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFDamageData.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDADamageProviderTestsPrivate
{
	// Synthetic Damage Create candidate가 사용하는 canonical target object path입니다.
	static const FString SyntheticTargetPath = TEXT("/Game/Test/CarFight/DDO/DA_Damage_ReadOnly.DA_Damage_ReadOnly");

	// Synthetic Damage candidate의 collision 가능성을 낮춘 stable identity입니다.
	static const FString SyntheticDamageId = TEXT("Damage_DDO_P0_01_ReadOnly");

	// DDO-P0-00에서 보호 대상으로 동결한 persisted Damage asset 이름입니다.
	static const TArray<FName> ProtectedDamageAssetNames =
	{
		FName(TEXT("DA_DamageAsset")),
		FName(TEXT("DA_DamageArmorPenTest"))
	};

	// DDO-P0-01 frozen exact12 Damage whole-record JSON을 생성합니다.
	FString BuildDamageJson(
		const FString& StableLogicalId = SyntheticDamageId,
		const FString& PayloadDamageId = SyntheticDamageId,
		const FString& TargetObjectPath = SyntheticTargetPath,
		const FString& BaseFingerprintJson = TEXT("null"),
		const FString& DamageTypeJson = TEXT("\"Kinetic\""),
		const FString& BaseDamageText = TEXT("25.0"),
		const FString& ArmorPenetrationText = TEXT("50.0"),
		const FString& MinExplosionDamageScaleText = TEXT("0.25"))
	{
		return FString::Printf(
			TEXT("{")
			TEXT("\"SchemaId\":\"CarFight.DataAsset.DamageData\",")
			TEXT("\"SchemaRevision\":1,")
			TEXT("\"AdapterContractRevision\":1,")
			TEXT("\"DataAssetTypeClassPath\":\"/Script/CarFight_Re.CFDamageData\",")
			TEXT("\"StableLogicalId\":\"%s\",")
			TEXT("\"TargetObjectPath\":\"%s\",")
			TEXT("\"BaseSemanticFingerprint\":%s,")
			TEXT("\"Payload\":{")
			TEXT("\"DamageId\":\"%s\",")
			TEXT("\"DamageType\":%s,")
			TEXT("\"BaseDamage\":%s,")
			TEXT("\"bCanDamageSelf\":false,")
			TEXT("\"ArmorPenetration\":%s,")
			TEXT("\"bUseRadialDamage\":true,")
			TEXT("\"ExplosionRadius\":200.0,")
			TEXT("\"ExplosionInnerRadius\":300.0,")
			TEXT("\"ExplosionDamage\":100.0,")
			TEXT("\"MinExplosionDamageScale\":%s,")
			TEXT("\"ModuleDamageScale\":1.0,")
			TEXT("\"ImpulseStrength\":500.0")
			TEXT("}")
			TEXT("}"),
			*StableLogicalId,
			*TargetObjectPath,
			*BaseFingerprintJson,
			*PayloadDamageId,
			*DamageTypeJson,
			*BaseDamageText,
			*ArmorPenetrationText,
			*MinExplosionDamageScaleText);
	}

	// Valid Damage record를 strict provider parser로 읽고 기본 assertion을 수행합니다.
	FCFDADamageParseResult ParseValidDamage(
		FAutomationTestBase& Test,
		const FString& JsonText,
		const FString& StagingPath = TEXT("Authoring/DataAssetStaging/DamageData/Damage_DDO_P0_01_ReadOnly.json"))
	{
		// Strict Damage parser 결과입니다.
		const FCFDADamageParseResult ParseResult = CFDADamageProviderImpl::ParseJson(JsonText, StagingPath);
		Test.TestTrue(TEXT("Expected Damage JSON must parse as valid exact12 whole-record"), ParseResult.bValid);
		Test.TestTrue(
			TEXT("Expected Damage staging fingerprint must be canonical sha256"),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ParseResult.Record.StagingSemanticFingerprint));
		return ParseResult;
	}

	// Current Asset Registry에서 protected Damage asset exact2를 이름 기준으로 찾습니다.
	TArray<FAssetData> FindProtectedDamageAssets()
	{
		// Current Project Asset Registry입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Exact UCFDamageData native class filter입니다.
		FARFilter DamageFilter;
		DamageFilter.ClassPaths.Add(UCFDamageData::StaticClass()->GetClassPathName());
		DamageFilter.bRecursiveClasses = false;

		// Current exact UCFDamageData metadata입니다.
		TArray<FAssetData> AllDamageAssets;
		(void)AssetRegistry.GetAssets(DamageFilter, AllDamageAssets, false);

		// DDO protected exact2 metadata만 담는 결과입니다.
		TArray<FAssetData> ProtectedAssets;
		for (const FAssetData& DamageAssetData : AllDamageAssets)
		{
			if (ProtectedDamageAssetNames.Contains(DamageAssetData.AssetName))
			{
				ProtectedAssets.Add(DamageAssetData);
			}
		}
		ProtectedAssets.Sort([](const FAssetData& Left, const FAssetData& Right)
		{
			return Left.AssetName.LexicalLess(Right.AssetName);
		});
		return ProtectedAssets;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageParseFingerprintTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_01.ParseFingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageNegativeContractTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_01.NegativeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamagePreviewProviderTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_01.PreviewProvider",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageProtectedAssetsTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_01.ProtectedAssetsReadOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Damage exact12 strict parse/serialize와 runtime-active ArmorPenetration fingerprint semantic을 검증합니다.
bool FCFDADamageParseFingerprintTest::RunTest(const FString& Parameters)
{
	using namespace CFDADamageProviderTestsPrivate;

	// Canonical valid Damage exact12 record입니다. InnerRadius > Radius를 의도적으로 사용해 새 교차제약이 없음을 함께 증명합니다.
	const FCFDADamageParseResult FirstResult = ParseValidDamage(*this, BuildDamageJson());
	TestEqual(TEXT("Damage exact12 parsed DamageType"), FirstResult.Record.Payload.DamageType, ECFDamageType::Kinetic);
	TestEqual(TEXT("Damage exact12 parsed ArmorPenetration"), FirstResult.Record.Payload.ArmorPenetration, 50.0f);
	TestTrue(
		TEXT("Existing runtime contract must allow ExplosionInnerRadius > ExplosionRadius in authoring payload"),
		FirstResult.Record.Payload.ExplosionInnerRadius > FirstResult.Record.Payload.ExplosionRadius);

	// FName case-only identity 차이만 가진 semantic-equivalent Damage record입니다.
	const FCFDADamageParseResult CaseEquivalentResult = ParseValidDamage(
		*this,
		BuildDamageJson(
			TEXT("damage_ddo_p0_01_readonly"),
			TEXT("damage_ddo_p0_01_readonly")));
	TestEqual(
		TEXT("FName case-only DamageId change must preserve semantic fingerprint"),
		FirstResult.Record.StagingSemanticFingerprint,
		CaseEquivalentResult.Record.StagingSemanticFingerprint);

	// Runtime armor path가 실제 소비하는 ArmorPenetration만 변경한 Damage record입니다.
	const FCFDADamageParseResult ArmorPenChangedResult = ParseValidDamage(
		*this,
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"Kinetic\""),
			TEXT("25.0"),
			TEXT("51.0")));
	TestNotEqual(
		TEXT("ArmorPenetration change must alter Damage semantic fingerprint"),
		FirstResult.Record.StagingSemanticFingerprint,
		ArmorPenChangedResult.Record.StagingSemanticFingerprint);

	// DamageType authored semantic만 변경한 Damage record입니다.
	const FCFDADamageParseResult DamageTypeChangedResult = ParseValidDamage(
		*this,
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"Energy\"")));
	TestNotEqual(
		TEXT("DamageType change must alter Damage semantic fingerprint"),
		FirstResult.Record.StagingSemanticFingerprint,
		DamageTypeChangedResult.Record.StagingSemanticFingerprint);

	// Typed payload에서 deterministic whole-record JSON을 생성합니다.
	FString SerializedJson;
	// Serializer failure detail입니다.
	FString SerializeError;
	TestTrue(
		TEXT("Damage exact12 serializer must accept valid typed payload"),
		CFDADamageProviderImpl::SerializeStagingJson(
			FirstResult.Record.Payload,
			FirstResult.Record.TargetObjectPath,
			FString(),
			SerializedJson,
			SerializeError));
	// Serializer 결과를 다시 strict parse한 record입니다.
	const FCFDADamageParseResult RoundTripResult = ParseValidDamage(*this, SerializedJson);
	TestEqual(
		TEXT("Damage serialize/parse round-trip must preserve semantic fingerprint"),
		FirstResult.Record.StagingSemanticFingerprint,
		RoundTripResult.Record.StagingSemanticFingerprint);
	TestFalse(TEXT("Serializer empty baseline must round-trip as Create intent"), RoundTripResult.Record.bHasBaseSemanticFingerprint);
	return true;
}

// Damage exact12 missing/unknown/type/enum/identity/range/revision/cached-fingerprint 입력을 fail-closed 검증합니다.
bool FCFDADamageNegativeContractTest::RunTest(const FString& Parameters)
{
	using namespace CFDADamageProviderTestsPrivate;

	// Unknown top-level field가 포함된 JSON입니다.
	FString UnknownFieldJson = BuildDamageJson();
	UnknownFieldJson.ReplaceInline(TEXT("\"SchemaId\":"), TEXT("\"Unexpected\":1,\"SchemaId\":"));
	// Unknown field parse 결과입니다.
	const FCFDADamageParseResult UnknownField = CFDADamageProviderImpl::ParseJson(UnknownFieldJson);
	TestFalse(TEXT("Unknown top-level field must be invalid"), UnknownField.bValid);
	TestTrue(TEXT("Unknown field diagnostic"), FCFDAStagingService::HasIssueCode(UnknownField.Issues, ECFDAStagingIssueCode::UnknownField));

	// Required exact12 field 하나를 제거한 JSON입니다.
	FString MissingFieldJson = BuildDamageJson();
	MissingFieldJson.ReplaceInline(TEXT("\"ArmorPenetration\":50.0,"), TEXT(""));
	// Missing exact12 field parse 결과입니다.
	const FCFDADamageParseResult MissingField = CFDADamageProviderImpl::ParseJson(MissingFieldJson);
	TestFalse(TEXT("Missing Damage exact12 field must be invalid"), MissingField.bValid);
	TestTrue(TEXT("Missing field diagnostic"), FCFDAStagingService::HasIssueCode(MissingField.Issues, ECFDAStagingIssueCode::MissingRequiredField));

	// DamageType physical JSON type을 number로 만든 record입니다.
	const FCFDADamageParseResult WrongEnumPhysicalType = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("1")));
	TestFalse(TEXT("Numeric DamageType must be rejected"), WrongEnumPhysicalType.bValid);
	TestTrue(TEXT("DamageType physical type diagnostic"), FCFDAStagingService::HasIssueCode(WrongEnumPhysicalType.Issues, ECFDAStagingIssueCode::TypeMismatch));

	// DamageType casing이 canonical token과 다른 record입니다.
	const FCFDADamageParseResult WrongEnumToken = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"kinetic\"")));
	TestFalse(TEXT("Non-canonical DamageType token must be rejected"), WrongEnumToken.bValid);
	TestTrue(TEXT("DamageType enum diagnostic"), FCFDAStagingService::HasIssueCode(WrongEnumToken.Issues, ECFDAStagingIssueCode::InvalidEnumValue));

	// Top-level StableLogicalId와 Payload.DamageId가 다른 record입니다.
	const FCFDADamageParseResult IdentityMismatch = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(TEXT("Damage_A"), TEXT("Damage_B")));
	TestFalse(TEXT("DamageId split authority must be invalid"), IdentityMismatch.bValid);
	TestTrue(TEXT("Damage identity mismatch diagnostic"), FCFDAStagingService::HasIssueCode(IdentityMismatch.Issues, ECFDAStagingIssueCode::StableIdentityMismatch));

	// Negative BaseDamage authored value입니다.
	const FCFDADamageParseResult NegativeBaseDamage = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"Kinetic\""),
			TEXT("-0.1")));
	TestFalse(TEXT("Negative BaseDamage must be rejected without clamp"), NegativeBaseDamage.bValid);
	TestTrue(TEXT("Negative BaseDamage diagnostic"), FCFDAStagingService::HasIssueCode(NegativeBaseDamage.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Frozen authored contract가 금지하는 exact zero BaseDamage record입니다.
	const FCFDADamageParseResult ZeroBaseDamage = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"Kinetic\""),
			TEXT("0.0")));
	TestFalse(TEXT("Zero BaseDamage must be rejected because authored contract requires > 0"), ZeroBaseDamage.bValid);
	TestTrue(TEXT("Zero BaseDamage diagnostic"), FCFDAStagingService::HasIssueCode(ZeroBaseDamage.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Radial damage를 활성화한 상태에서 ExplosionRadius만 exact zero로 만든 record입니다.
	FString ZeroRadialRadiusJson = BuildDamageJson();
	ZeroRadialRadiusJson.ReplaceInline(TEXT("\"ExplosionRadius\":200.0"), TEXT("\"ExplosionRadius\":0.0"));
	// Radial-enabled zero radius parse 결과입니다.
	const FCFDADamageParseResult ZeroRadialRadius = CFDADamageProviderImpl::ParseJson(ZeroRadialRadiusJson);
	TestFalse(TEXT("Radial-enabled Damage must reject zero ExplosionRadius"), ZeroRadialRadius.bValid);
	TestTrue(TEXT("Radial-enabled zero radius diagnostic"), FCFDAStagingService::HasIssueCode(ZeroRadialRadius.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Radial damage를 활성화한 상태에서 ExplosionDamage만 exact zero로 만든 record입니다.
	FString ZeroRadialDamageJson = BuildDamageJson();
	ZeroRadialDamageJson.ReplaceInline(TEXT("\"ExplosionDamage\":100.0"), TEXT("\"ExplosionDamage\":0.0"));
	// Radial-enabled zero damage parse 결과입니다.
	const FCFDADamageParseResult ZeroRadialDamage = CFDADamageProviderImpl::ParseJson(ZeroRadialDamageJson);
	TestFalse(TEXT("Radial-enabled Damage must reject zero ExplosionDamage"), ZeroRadialDamage.bValid);
	TestTrue(TEXT("Radial-enabled zero damage diagnostic"), FCFDAStagingService::HasIssueCode(ZeroRadialDamage.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Negative ArmorPenetration authored value입니다.
	const FCFDADamageParseResult NegativeArmorPenetration = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"Kinetic\""),
			TEXT("25.0"),
			TEXT("-1.0")));
	TestFalse(TEXT("Negative ArmorPenetration must be rejected without clamp"), NegativeArmorPenetration.bValid);
	TestTrue(TEXT("Negative ArmorPenetration diagnostic"), FCFDAStagingService::HasIssueCode(NegativeArmorPenetration.Issues, ECFDAStagingIssueCode::InvalidValue));

	// MinExplosionDamageScale authored range를 넘긴 record입니다.
	const FCFDADamageParseResult InvalidMinScale = CFDADamageProviderImpl::ParseJson(
		BuildDamageJson(
			SyntheticDamageId,
			SyntheticDamageId,
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("\"Kinetic\""),
			TEXT("25.0"),
			TEXT("50.0"),
			TEXT("1.1")));
	TestFalse(TEXT("MinExplosionDamageScale > 1 must be rejected"), InvalidMinScale.bValid);
	TestTrue(TEXT("MinExplosionDamageScale diagnostic"), FCFDAStagingService::HasIssueCode(InvalidMinScale.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Wrong exact native class TypeKey입니다.
	FString WrongClassPathJson = BuildDamageJson();
	WrongClassPathJson.ReplaceInline(TEXT("/Script/CarFight_Re.CFDamageData"), TEXT("/Script/CarFight_Re.CFAmmoData"));
	// Wrong class path parse 결과입니다.
	const FCFDADamageParseResult WrongClassPath = CFDADamageProviderImpl::ParseJson(WrongClassPathJson);
	TestFalse(TEXT("Wrong Damage DataAssetTypeClassPath must fail exact TypeKey lookup"), WrongClassPath.bValid);
	TestTrue(TEXT("Wrong Damage TypeKey diagnostic"), FCFDAStagingService::HasIssueCode(WrongClassPath.Issues, ECFDAStagingIssueCode::SchemaUnsupported));

	// Wrong Damage SchemaRevision입니다.
	FString WrongSchemaRevisionJson = BuildDamageJson();
	WrongSchemaRevisionJson.ReplaceInline(TEXT("\"SchemaRevision\":1"), TEXT("\"SchemaRevision\":2"));
	// Wrong Damage SchemaRevision parse 결과입니다.
	const FCFDADamageParseResult WrongSchemaRevision = CFDADamageProviderImpl::ParseJson(WrongSchemaRevisionJson);
	TestFalse(TEXT("Wrong Damage SchemaRevision must be rejected"), WrongSchemaRevision.bValid);
	TestTrue(TEXT("Wrong Damage SchemaRevision diagnostic"), FCFDAStagingService::HasIssueCode(WrongSchemaRevision.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported));

	// Valid record의 typed payload만 mutate해 cached fingerprint와 split한 fixture입니다.
	FCFDADamageParseResult SplitRecordResult = ParseValidDamage(*this, BuildDamageJson());
	SplitRecordResult.Record.Payload.ArmorPenetration += 1.0f;
	// Split-state validation failure detail입니다.
	FString SplitIntegrityError;
	TestFalse(
		TEXT("Damage provider-local payload/cached fingerprint split must fail-closed"),
		CFDADamageProviderImpl::ValidateRecordIntegrity(SplitRecordResult.Record, SplitIntegrityError));

	// Non-finite typed BaseDamage fingerprint fixture입니다.
	FCFDADamagePayload NonFinitePayload = SplitRecordResult.Record.Payload;
	NonFinitePayload.BaseDamage = std::numeric_limits<float>::infinity();
	// Non-finite fingerprint output입니다.
	FString NonFiniteFingerprint;
	// Non-finite fingerprint failure detail입니다.
	FString NonFiniteError;
	TestFalse(
		TEXT("Non-finite typed Damage value must not fingerprint"),
		CFDADamageProviderImpl::BuildSemanticFingerprint(NonFinitePayload, NonFiniteFingerprint, NonFiniteError));
	return true;
}

// Damage production provider의 Reviewed mutation/DACE readiness와 shared Create Preview capability를 검증합니다.
bool FCFDADamagePreviewProviderTest::RunTest(const FString& Parameters)
{
	using namespace CFDADamageProviderTestsPrivate;

	// Production exact3 provider registry validation detail입니다.
	FString RegistryError;
	TestTrue(TEXT("Missile + Ammo + Damage production provider registry must validate"), CFDATypeDispatch::ValidateProviderRegistry(RegistryError));

	// Damage exact TypeKey lookup detail입니다.
	FString LookupError;
	// Damage exact production provider entry입니다.
	const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		TEXT("CarFight.DataAsset.DamageData"),
		TEXT("/Script/CarFight_Re.CFDamageData"),
		&LookupError);
	TestTrue(TEXT("Damage exact TypeKey must resolve production provider"), ProviderEntry == &CFDADamageProvider::GetProvider());
	if (ProviderEntry == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Damage provider readiness must be ReviewedMutationReady"), ProviderEntry->Readiness, ECFDAProviderReadiness::ReviewedMutationReady);
	TestTrue(TEXT("Damage ParseCommonCandidate callback must exist"), ProviderEntry->Operations.ParseCommonCandidate != nullptr);
	TestTrue(TEXT("Damage ResolveCommonCurrentState callback must exist"), ProviderEntry->Operations.ResolveCommonCurrentState != nullptr);
	TestTrue(TEXT("Damage ApplyReviewedMutation callback must exist"), ProviderEntry->Operations.ApplyReviewedMutation != nullptr);
	TestEqual(TEXT("Damage DACE readiness must be ContractReady after DDO-P0-03 activation transaction"), ProviderEntry->Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestTrue(TEXT("Damage DACE canonical target-set declaration must be explicit"), ProviderEntry->Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Product canonical Damage Staging target set must remain exact0"), ProviderEntry->Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);

	// Reviewed mutation readiness validation detail입니다.
	FString MutationReadyError;
	TestTrue(
		TEXT("Damage provider must keep Reviewed mutation readiness after P0-03 DACE activation"),
		CFDATypeDispatch::ValidateProviderMutationReady(*ProviderEntry, MutationReadyError));

	// Provider-local direct Damage parse가 Preview용으로 계속 가능한 candidate입니다.
	FCFDACommonEnvelope DamageEnvelope;
	// Provider-local Damage parse diagnostics입니다.
	TArray<FCFDAStagingIssue> ParseIssues;
	TestTrue(
		TEXT("Damage provider must parse exact12 candidate for read-only Preview"),
		ProviderEntry->Operations.ParseCommonCandidate(
			BuildDamageJson(),
			TEXT("Authoring/DataAssetStaging/DamageData/Damage_DDO_P0_01_ReadOnly.json"),
			DamageEnvelope,
			ParseIssues));

	// Synthetic absent Damage target current truth입니다.
	FCFDACommonCurrentState CurrentState;
	// Current read-only resolver diagnostics입니다.
	TArray<FCFDAStagingIssue> CurrentIssues;
	TestTrue(
		TEXT("Synthetic absent Damage current truth must resolve read-only"),
		ProviderEntry->Operations.ResolveCommonCurrentState(DamageEnvelope, CurrentState, CurrentIssues));
	TestFalse(TEXT("Synthetic Damage requested target must remain absent"), CurrentState.bRequestedTargetExists);

	// Existing shared Preview state machine을 그대로 재사용한 Damage row입니다.
	const FCFDACommonPreviewRow PreviewRow = CFDATypeDispatch::BuildCommonPreview(DamageEnvelope, CurrentState);
	TestEqual(TEXT("Absent exact12 Damage candidate must classify as Create in shared Preview"), PreviewRow.Kind, ECFDAStagingPreviewKind::Create);
	TestEqual(TEXT("Shared Damage Preview must not introduce blocking issues"), PreviewRow.Issues.Num(), 0);
	return true;
}

// DDO-P0-00 protected DA_DamageAsset/DA_DamageArmorPenTest exact2를 mutation 없이 extractor/current fingerprint로 읽습니다.
bool FCFDADamageProtectedAssetsTest::RunTest(const FString& Parameters)
{
	using namespace CFDADamageProviderTestsPrivate;

	// Asset Registry에서 찾은 protected Damage exact2입니다.
	const TArray<FAssetData> ProtectedAssets = FindProtectedDamageAssets();
	TestEqual(TEXT("Protected persisted Damage assets must remain exact2"), ProtectedAssets.Num(), 2);
	if (ProtectedAssets.Num() != 2)
	{
		return false;
	}

	for (const FAssetData& ProtectedAssetData : ProtectedAssets)
	{
		// Read-only load한 exact protected UCFDamageData입니다.
		UCFDamageData* ProtectedDamage = Cast<UCFDamageData>(ProtectedAssetData.GetAsset());
		TestNotNull(TEXT("Protected Damage metadata must load as UCFDamageData"), ProtectedDamage);
		if (ProtectedDamage == nullptr)
		{
			continue;
		}

		// Read-only extraction 전 owning package입니다.
		UPackage* ProtectedPackage = ProtectedDamage->GetOutermost();
		TestNotNull(TEXT("Protected Damage must have owning package"), ProtectedPackage);
		if (ProtectedPackage == nullptr)
		{
			continue;
		}
		// 이 test가 변경하면 안 되는 pre-existing package dirty 상태입니다.
		const bool bPackageDirtyBefore = ProtectedPackage->IsDirty();

		// Persisted Damage exact12 payload입니다.
		FCFDADamagePayload PersistedPayload;
		// Extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		TestTrue(
			TEXT("Protected Damage exact12 extractor must succeed"),
			CFDADamageProviderImpl::ExtractPayload(*ProtectedDamage, PersistedPayload, ExtractIssues));
		TestFalse(TEXT("Protected DamageId must remain required/non-None"), PersistedPayload.DamageId.IsNone());

		// Persisted exact12 semantic fingerprint입니다.
		FString PersistedFingerprint;
		// Fingerprint generation failure detail입니다.
		FString FingerprintError;
		TestTrue(
			TEXT("Protected Damage semantic fingerprint must succeed"),
			CFDADamageProviderImpl::BuildSemanticFingerprint(PersistedPayload, PersistedFingerprint, FingerprintError));
		TestTrue(
			TEXT("Protected Damage fingerprint must be canonical sha256"),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(PersistedFingerprint));

		// Current resolver용 provider-local read-only record입니다.
		FCFDADamageRecord CurrentRecord;
		CurrentRecord.SchemaId = CFDADamageProvider::GetProvider().Descriptor.TypeKey.SchemaId;
		CurrentRecord.SchemaRevision = CFDADamageProvider::GetProvider().Descriptor.SchemaRevision;
		CurrentRecord.AdapterContractRevision = CFDADamageProvider::GetProvider().Descriptor.AdapterContractRevision;
		CurrentRecord.DataAssetTypeClassPath = CFDADamageProvider::GetProvider().Descriptor.TypeKey.DataAssetTypeClassPath;
		CurrentRecord.StableLogicalId = PersistedPayload.DamageId;
		CurrentRecord.TargetObjectPath = ProtectedAssetData.GetSoftObjectPath().ToString();
		CurrentRecord.Payload = PersistedPayload;
		CurrentRecord.StagingSemanticFingerprint = PersistedFingerprint;
		CurrentRecord.StagingRelativePath = FString::Printf(
			TEXT("Authoring/DataAssetStaging/DamageData/%s.json"),
			*ProtectedAssetData.AssetName.ToString());

		// Persisted target envelope입니다.
		const FCFDACommonEnvelope CurrentEnvelope = CFDADamageProviderImpl::BuildCommonEnvelope(CurrentRecord);
		// Persisted target current readback입니다.
		FCFDACommonCurrentState CurrentState;
		// Current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		TestTrue(
			TEXT("Protected Damage current resolver must succeed read-only"),
			CFDADamageProviderImpl::ResolveCommonCurrentState(CurrentEnvelope, CurrentState, CurrentIssues));
		TestTrue(TEXT("Protected Damage requested target must exist"), CurrentState.bRequestedTargetExists);
		TestEqual(TEXT("Protected Damage current class path"), CurrentState.RequestedTargetClassPath, FString(TEXT("/Script/CarFight_Re.CFDamageData")));
		TestEqual(TEXT("Protected Damage current stable identity"), CurrentState.RequestedTargetStableLogicalId, PersistedPayload.DamageId);
		TestEqual(TEXT("Protected Damage current fingerprint"), CurrentState.CurrentSemanticFingerprint, PersistedFingerprint);
		TestEqual(TEXT("Protected Damage package dirty state must remain unchanged"), ProtectedPackage->IsDirty(), bPackageDirtyBefore);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
