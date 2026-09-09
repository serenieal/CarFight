// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractGuard.h
// Version: v1.3.0
// Date: 2026-09-09
// Description: CF-FQ-050 DACE-P0-04 MissileGuidePreset contract evolution, canonical Staging compatibility와 migration promotion gate의 Private 검증 계약입니다.
// Changelog:
// - v1.3.0: canonical Product Staging exact3 read-only strict-parse compatibility, migration Resolution/Evidence, Staging/Product Pending과 accepted append/Current promotion gate를 추가했습니다.
// - v1.2.1: Source/Mapping-only change를 자동 safe 판정하지 않고 explicit NoMigration의 structural admissibility만 검증하도록 교정했습니다. no-delta stale declaration과 malformed accepted component signature를 fail-closed합니다.
// - v1.2.0: CurrentChangeDeclaration, combined candidate contract signature, Schema/Adapter revision guard, explicit NoMigration safe-refactor gate와 accepted snapshot chain validator를 추가.
// - v1.1.0: Native Reflection Source descriptor, developer-only guard result, Adapter/mapping/serializer/fingerprint/materializer coverage validator와 fingerprint token descriptor를 추가.
// - v1.0.0: SourceShape/AdapterShape/SourceAdapterMapping/SemanticContract descriptor, bootstrap accepted snapshot chain, test-only production probe entry를 최초 추가.
// Migration:
// - Public/Blueprint API는 추가하지 않습니다. 이 header는 CarFight_ReEditor Private 전용이며 Product Sync/Apply/Save 권한을 소유하지 않습니다.
// - DACE-P0-02 validator는 current Product class를 수정하지 않고 Reflection과 memory/transient probe 결과만 비교합니다.
// - DACE-P0-03의 current change declaration은 accepted history와 분리하며, 실제 contract 변화가 없으면 declaration은 없어야 합니다. Revision Guard는 Product Sync/Apply/Save를 호출하지 않습니다.
// - v1.2.1부터 explicit NoMigration은 개발자의 safe 판단이며 Guard가 Source/Mapping-only category만으로 자동 추론하지 않습니다. accepted four component signature는 canonical lowercase SHA-256이어야 합니다.
// - v1.3.0부터 Guard는 canonical Product Staging을 read-only로만 읽고 strict parser에 전달합니다. Pending/invalid migration은 accepted append와 Current System promotion을 허용하지 않으며 SyncProduct/ApplyReviewed/SavePackage를 호출하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFDAStaging.h"

class UCFMissileGuidePresetData;

/** 개발자 전용 Data Asset contract guard 진단 코드입니다. */
enum class ECFDAContractIssueCode : uint8
{
	None,
	SourceAuthoringContractDrift,
	AdapterShapeDrift,
	SourceAdapterMappingDrift,
	SerializerCoverageMismatch,
	ParserCoverageMismatch,
	FingerprintCoverageMismatch,
	MaterializerCoverageMismatch,
	SemanticBehaviorMismatch,
	SchemaRevisionBumpRequired,
	AdapterRevisionBumpRequired,
	MigrationImpactUndeclared,
	StagingMigrationPending,
	ProductMigrationReviewPending,
	AcceptedSnapshotChainInvalid
};

/** Contract 변화가 요구하는 migration 범위입니다. */
enum class ECFDAContractMigrationImpact : uint8
{
	NoMigration,
	StagingMigrationRequired,
	ProductMigrationReviewRequired,
	StagingAndProductMigrationReviewRequired
};

/** Declared migration의 현재 해결 상태입니다. */
enum class ECFDAContractMigrationResolution : uint8
{
	NotRequired,
	Pending,
	Resolved
};

/** Native Source authored property 한 노드의 안정적 structural descriptor입니다. */
struct FCFDASourceFieldDescriptor
{
	// Canonical Source root class path입니다.
	FString SourceRootClassPath;
	// Root부터 시작하는 canonical authored property path입니다.
	FString SourcePropertyPath;
	// Bool/Float/Enum/Name/Text/Struct 중 안정적 property kind입니다.
	FString PropertyKind;
	// Enum/Struct 등 reflected type이 있는 경우 canonical type path입니다.
	FString ReflectedTypePath;
	// Scalar/Struct/Array/Set/Map 중 container shape입니다.
	FString ContainerKind;
	// Nested struct node인 경우 canonical struct path입니다.
	FString NestedStructPath;
};

/** Strict Staging JSON field 한 노드의 physical shape descriptor입니다. */
struct FCFDAAdapterFieldDescriptor
{
	// Root부터 시작하는 canonical JSON field path입니다.
	FString AdapterJsonPath;
	// Object/String/Number/Boolean/StringOrNull 중 JSON value kind입니다.
	FString JsonValueKind;
	// Required/Optional field presence policy입니다.
	FString PresencePolicy;
	// NonNull/Nullable null policy입니다.
	FString NullPolicy;
	// NameToken/LiteralFText/EnumNameToken/Float/Bool 등 representation policy입니다.
	FString RepresentationKind;
	// Metadata/Identity/Payload/PayloadContainer/RepresentationConstant 중 semantic role입니다.
	FString SemanticRole;
};

/** Source authored leaf와 Adapter semantic field의 explicit mapping descriptor입니다. */
struct FCFDASourceAdapterMappingDescriptor
{
	// Source authored property path이며 Adapter-only metadata는 empty입니다.
	FString SourcePropertyPath;
	// 대응하는 Adapter JSON field path입니다.
	FString AdapterJsonPath;
	// NameToken/LiteralText/LiteralKind/EnumNameToken/Float/Bool/Metadata 중 mapping representation입니다.
	FString RepresentationKind;
	// SourceToAdapter/AdapterOnlyMetadata/AdapterOnlyConstant 중 mapping role입니다.
	FString MappingRole;
};

/** Reflection만으로 얻을 수 없는 explicit semantic policy descriptor입니다. */
struct FCFDASemanticRuleDescriptor
{
	// Stable semantic policy key입니다.
	FString PolicyKey;
	// Revisioned semantic policy value입니다.
	FString PolicyValue;
};

/** Production semantic fingerprint가 emit해야 하는 한 contract token입니다. */
struct FCFDAFingerprintTokenDescriptor
{
	// Production fingerprint stream의 exact token label입니다.
	FString TokenLabel;
	// 이 token이 대표하는 Source authored property path이며 metadata token은 empty입니다.
	FString SourcePropertyPath;
	// Metadata/PayloadSemanticLeaf 중 token contract role입니다.
	FString TokenRole;
};

/** Current contract의 deterministic component signatures입니다. */
struct FCFDAContractSignatures
{
	// Native authored Source structural signature입니다.
	FString SourceShapeSignature;
	// Strict Staging JSON physical shape signature입니다.
	FString AdapterShapeSignature;
	// Explicit Source↔Adapter mapping signature입니다.
	FString SourceAdapterMappingSignature;
	// Explicit semantic policy signature입니다.
	FString SemanticContractSignature;
};

/** Accepted 전 current contract change의 explicit migration declaration입니다. */
struct FCFDACurrentChangeDeclaration
{
	// 반드시 latest accepted snapshot을 가리키는 base identity입니다.
	FString BaseSnapshotId;
	// Source/Adapter/Mapping/Semantic four-signature 묶음에서 Guard가 계산하는 candidate signature입니다.
	FString CandidateContractSignature;
	// NoMigration을 default/undeclared와 구분하기 위한 explicit declaration bit입니다.
	bool bImpactDeclared = false;
	// 선언된 migration impact입니다.
	ECFDAContractMigrationImpact Impact = ECFDAContractMigrationImpact::NoMigration;
	// 선언된 migration resolution입니다.
	ECFDAContractMigrationResolution Resolution = ECFDAContractMigrationResolution::NotRequired;
	// Resolved migration evidence identity입니다.
	FString MigrationEvidenceId;
};

/** Production Editor module이 소유하는 accepted contract history record입니다. */
struct FCFDAAcceptedContractSnapshot
{
	// Append-only snapshot stable identity입니다.
	FString SnapshotId;
	// Previous accepted snapshot record signature이며 bootstrap은 empty입니다.
	FString PreviousSnapshotSignature;
	// Accepted schema identity입니다.
	FString SchemaId;
	// Accepted JSON physical shape revision입니다.
	int32 SchemaRevision = 0;
	// Accepted typed adapter semantic revision입니다.
	int32 AdapterContractRevision = 0;
	// Accepted native DataAsset class path입니다.
	FString DataAssetTypeClassPath;
	// Accepted Source structural signature입니다.
	FString SourceShapeSignature;
	// Accepted Adapter physical shape signature입니다.
	FString AdapterShapeSignature;
	// Accepted Source↔Adapter mapping signature입니다.
	FString SourceAdapterMappingSignature;
	// Accepted explicit semantic contract signature입니다.
	FString SemanticContractSignature;
	// Accepted migration impact입니다.
	ECFDAContractMigrationImpact MigrationImpact = ECFDAContractMigrationImpact::NoMigration;
	// Accepted migration resolution입니다.
	ECFDAContractMigrationResolution MigrationResolution = ECFDAContractMigrationResolution::NotRequired;
	// Resolved migration evidence identity이며 bootstrap NoMigration에서는 empty입니다.
	FString MigrationEvidenceId;
	// 이 record 자체의 deterministic chain signature입니다.
	FString SnapshotSignature;
};

/** Developer-only contract guard 한 건의 machine-readable 진단입니다. */
struct FCFDAContractGuardIssue
{
	// Stable developer guard issue code입니다.
	ECFDAContractIssueCode Code = ECFDAContractIssueCode::None;
	// 가능한 경우 drift가 발생한 source/adapter path입니다.
	FString FieldPath;
	// 개발자가 읽을 수 있는 구체적인 실패 설명입니다.
	FString Message;
};

/** Developer-only contract guard의 fail-closed validation 결과입니다. */
struct FCFDAContractGuardResult
{
	// 모든 검사가 통과했는지 나타냅니다.
	bool bPassed = true;
	// 실패 원인을 machine-readable code와 함께 보존합니다.
	TArray<FCFDAContractGuardIssue> Issues;
};

/** Migration 상태가 accepted snapshot append와 Current System promotion을 허용하는지 나타내는 Private gate 결과입니다. */
struct FCFDAMigrationGateResult
{
	// Revision/strict parse/Resolution/Evidence를 합친 fail-closed validation입니다.
	FCFDAContractGuardResult Validation;
	// 현재 candidate를 새 accepted snapshot으로 append할 수 있는지 나타냅니다.
	bool bAcceptedSnapshotAppendAllowed = false;
	// 현재 contract를 Current System으로 promotion할 수 있는지 나타냅니다.
	bool bCurrentSystemPromotionAllowed = false;
};

/** DACE contract descriptor, Reflection drift와 structural execution coverage를 제공하는 Private service입니다. */
class FCFDAContractGuard
{
public:
	// Current MissileGuidePreset Source structural descriptor를 deterministic order로 반환합니다.
	static const TArray<FCFDASourceFieldDescriptor>& GetSourceShapeDescriptor();
	// Current MissileGuidePreset Adapter physical shape descriptor를 deterministic order로 반환합니다.
	static const TArray<FCFDAAdapterFieldDescriptor>& GetAdapterShapeDescriptor();
	// Current MissileGuidePreset explicit Source↔Adapter mapping descriptor를 deterministic order로 반환합니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor>& GetSourceAdapterMappingDescriptor();
	// Current MissileGuidePreset explicit semantic contract descriptor를 deterministic order로 반환합니다.
	static const TArray<FCFDASemanticRuleDescriptor>& GetSemanticContractDescriptor();
	// Current production semantic fingerprint가 emit해야 하는 exact token descriptor를 반환합니다.
	static const TArray<FCFDAFingerprintTokenDescriptor>& GetFingerprintTokenDescriptor();
	// Current four component descriptor signatures를 계산합니다.
	static bool BuildCurrentSignatures(FCFDAContractSignatures& OutSignatures, FString& OutError);
	// Four component signatures를 fixed-key order의 candidate contract signature 하나로 결합합니다.
	static bool BuildContractBundleSignature(const FCFDAContractSignatures& Signatures, FString& OutSignature, FString& OutError);
	// Native Reflection에서 current Staging-owned Source shape를 직접 관측합니다.
	static bool BuildReflectedSourceShapeDescriptor(TArray<FCFDASourceFieldDescriptor>& OutDescriptors, FString& OutError);
	// Expected Source descriptor와 observed Reflection/fixture descriptor를 exact 비교합니다.
	static FCFDAContractGuardResult ValidateSourceShapeCoverage(const TArray<FCFDASourceFieldDescriptor>& ExpectedDescriptors, const TArray<FCFDASourceFieldDescriptor>& ObservedDescriptors);
	// Expected Adapter descriptor와 observed/fixture Adapter descriptor를 exact 비교합니다.
	static FCFDAContractGuardResult ValidateAdapterShapeCoverage(const TArray<FCFDAAdapterFieldDescriptor>& ExpectedDescriptors, const TArray<FCFDAAdapterFieldDescriptor>& ObservedDescriptors);
	// Source leaf set, Adapter terminal field set과 mapping 양쪽 coverage가 exact인지 검증합니다.
	static FCFDAContractGuardResult ValidateSourceAdapterMappingCoverage(const TArray<FCFDASourceFieldDescriptor>& SourceDescriptors, const TArray<FCFDAAdapterFieldDescriptor>& AdapterDescriptors, const TArray<FCFDASourceAdapterMappingDescriptor>& MappingDescriptors);
	// Production serializer 또는 memory fixture JSON physical shape가 Adapter descriptor와 exact인지 검증합니다.
	static FCFDAContractGuardResult ValidateSerializedAdapterCoverage(const FString& JsonText);
	// Production fingerprint가 emit한 token label sequence가 fingerprint descriptor와 exact인지 검증합니다.
	static FCFDAContractGuardResult ValidateFingerprintCoverage(const TArray<FString>& ObservedTokenLabels);
	// Materializer→extractor readback payload가 expected payload의 full semantic fingerprint를 보존했는지 검증합니다.
	static FCFDAContractGuardResult ValidateMaterializerRoundTripCoverage(const FCFDAMissilePresetPayload& ExpectedPayload, const FCFDAMissilePresetPayload& ReadbackPayload);
	// Current native Reflection과 Source/Adapter/mapping descriptor 관계를 production-side structural gate로 검증합니다.
	static FCFDAContractGuardResult ValidateCurrentStructuralCoverage();
	// Guard result에 requested developer issue code가 존재하는지 확인합니다.
	static bool HasIssueCode(const FCFDAContractGuardResult& Result, ECFDAContractIssueCode Code);
	// Production-owned accepted snapshot append-only history를 반환합니다.
	static const TArray<FCFDAAcceptedContractSnapshot>& GetAcceptedSnapshots();
	// 아직 accepted되지 않은 current change declaration을 반환하며 current contract가 baseline과 같으면 nullptr입니다.
	static const FCFDACurrentChangeDeclaration* GetCurrentChangeDeclaration();
	// Latest accepted snapshot 대비 candidate revision/signature와 explicit migration declaration을 fail-closed 검증합니다.
	static FCFDAContractGuardResult ValidateRevisionGuard(const FCFDAAcceptedContractSnapshot& BaseSnapshot, const FCFDAContractSignatures& CandidateSignatures, int32 CandidateSchemaRevision, int32 CandidateAdapterContractRevision, const FCFDACurrentChangeDeclaration* ChangeDeclaration);
	// Accepted snapshot history의 four component signature integrity, record signature, unique id, previous chain과 revision monotonicity를 검증합니다.
	static FCFDAContractGuardResult ValidateAcceptedSnapshotChain(const TArray<FCFDAAcceptedContractSnapshot>& Snapshots);
	// Production accepted history와 actual current descriptor/service revision을 함께 검증합니다.
	static FCFDAContractGuardResult ValidateCurrentRevisionGuard();
	// 한 Staging JSON을 current production strict parser/revision 계약으로 read-only 검증합니다.
	static FCFDAContractGuardResult ValidateStagingJsonCompatibility(const FString& JsonText, const FString& StagingRelativePath);
	// canonical Product MissileGuidePreset Staging exact3을 disk에서 read-only로 읽어 current strict parser/revision과 검증합니다.
	static FCFDAContractGuardResult ValidateCurrentCanonicalStagingCompatibility();
	// Declared impact의 Resolution/Evidence와 canonical Staging 호환성을 migration pending semantics로 검증합니다.
	static FCFDAContractGuardResult ValidateMigrationResolution(ECFDAContractMigrationImpact Impact, ECFDAContractMigrationResolution Resolution, const FString& MigrationEvidenceId, bool bCanonicalStagingCompatible);
	// CurrentChangeDeclaration과 canonical Staging 결과를 accepted append/Current promotion machine gate로 평가합니다.
	static FCFDAMigrationGateResult EvaluateMigrationGate(const FCFDACurrentChangeDeclaration* ChangeDeclaration, const FCFDAContractGuardResult& CanonicalStagingResult);
	// Production revision guard + canonical exact3 + current migration declaration을 종합한 current operational gate를 평가합니다.
	static FCFDAMigrationGateResult EvaluateCurrentMigrationGate();
	// Historical first bootstrap record의 frozen identity와 전체 append-only accepted snapshot chain을 검증합니다.
	static bool ValidateBootstrapAcceptedSnapshot(FString& OutError);
	// Snapshot record의 chain signature를 deterministic하게 계산합니다.
	static bool BuildSnapshotSignature(const FCFDAAcceptedContractSnapshot& Snapshot, FString& OutSignature, FString& OutError);
};

#if WITH_DEV_AUTOMATION_TESTS
// Production serializer implementation을 직접 호출해 canonical JSON text를 관측합니다.
bool CFDAContractProbeSerialize(const FCFDAMissilePresetPayload& Payload, const FString& TargetObjectPath, const FString& BaseSemanticFingerprint, FString& OutJsonText, FString& OutError);
// Production strict parser implementation을 직접 호출합니다.
FCFDAStagingParseResult CFDAContractProbeParse(const FString& JsonText, const FString& StagingRelativePath);
// Production fingerprint implementation을 호출하면서 실제 semantic token label sequence를 관측합니다.
bool CFDAContractProbeFingerprint(const FCFDAMissilePresetPayload& Payload, FString& OutFingerprint, TArray<FString>& OutTokenLabels, FString& OutError);
// Production extractor implementation을 직접 호출합니다.
bool CFDAContractProbeExtract(const UCFMissileGuidePresetData& Asset, FCFDAMissilePresetPayload& OutPayload, TArray<FCFDAStagingIssue>& OutIssues);
// Production materializer를 test-owned transient asset에 적용한 뒤 production extractor로 readback합니다.
bool CFDAContractProbeMaterializeRoundTrip(const FCFDAMissilePresetPayload& Payload, FCFDAMissilePresetPayload& OutReadbackPayload, FString& OutError);
#endif
