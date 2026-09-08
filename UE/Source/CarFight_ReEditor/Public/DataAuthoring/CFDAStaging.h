// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStaging.h
// Version: v1.3.1
// Date: 2026-09-08
// Description: CF-FQ-049 DAS-P0-02 typed DataAsset Staging parse/fingerprint/Preview foundation public contract입니다.
// Changelog:
// - v1.3.1: Product AssetDump의 NSLOCTEXT("", generated-key, source) evidence에 맞춰 source-backed empty-authored-namespace FText의 stable key를 persistence metadata로 canonicalize하는 exact P0 Literal 경계를 확정.
// - v1.3.0: DAS-P0-04 persisted round-trip에서 UE stable localization key가 부여한 package-only namespace/key는 source-backed Literal 의미로 canonicalize하고, StringTable/explicit authored namespace/source-less representation은 계속 fail-closed하는 FText semantic contract를 확정.
// - v1.2.0: current resolver와 Apply/rollback/post-save validation이 동일한 UObject→typed whole-record semantic extractor를 재사용할 수 있도록 exact MissileGuidePreset extractor를 public service contract로 승격.
// - v1.1.2: Preview가 mutable TargetObjectPath를 재검증하고 BatchPlanHash가 canonical StagingRelativePath와 Create/Update intent binding을 다시 검증하는 defense-in-depth 계약을 명확히 함.
// - v1.1.1: 구현 교정에 맞춰 Staging source authority와 Payload↔fingerprint integrity를 Preview/BatchPlanHash가 fail-closed한다는 public contract 주석을 명확히 함.
// - v1.1.0: MissileGuidePreset exact target/StableIdentity를 Asset Registry + typed payload로 읽는 read-only current resolver와 current duplicate count 계약을 추가.
// - v1.0.0: MissileGuidePreset strict whole-record DTO, stable diagnostics, semantic fingerprint, mutation0 Preview와 BatchPlanHash 계약을 추가.
// Migration:
// - Editor-only read/parse/compare foundation입니다. UObject 생성/수정, package dirty 변경, Asset Registry mutation, SavePackage를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileGuideTypes.h"

// Staging 한 record의 mutation0 Preview 최종 분류입니다.
enum class ECFDAStagingPreviewKind : uint8
{
	Create,
	Update,
	NoChange,
	Conflict,
	Invalid
};

// P0-01에서 동결한 machine-readable Staging/Preview 진단 코드입니다.
enum class ECFDAStagingIssueCode : uint8
{
	None,
	MalformedJson,
	SchemaUnsupported,
	SchemaRevisionUnsupported,
	AdapterRevisionMismatch,
	MissingRequiredField,
	UnknownField,
	TypeMismatch,
	InvalidValue,
	UnsupportedTextRepresentation,
	InvalidEnumValue,
	StableIdentityMissing,
	StableIdentityMismatch,
	DuplicateStableIdentity,
	DuplicateTargetPath,
	BaselineMissing,
	BaselineMismatch,
	BaselineRebaseRequired,
	UnexpectedExistingTarget,
	TargetMissing,
	TargetMoved,
	PathCollision,
	TargetDirtyUnowned,
	ApprovalStale
};

// 한 parse/Preview 진단의 machine code와 사용자 설명을 함께 보존합니다.
struct FCFDAStagingIssue
{
	// machine-readable stable diagnostic code입니다.
	ECFDAStagingIssueCode Code = ECFDAStagingIssueCode::None;

	// 문제가 발생한 JSON 또는 Preview field 경로입니다.
	FString FieldPath;

	// 사용자가 읽을 수 있는 bounded 한국어 진단 메시지입니다.
	FString Message;

	// true이면 현재 record가 approval/mutation candidate가 될 수 없는 blocking issue입니다.
	bool bBlocking = true;
};

// P0 Literal FText interchange를 localization identity와 분리해 보존하는 typed DTO입니다.
struct FCFDAStagingLiteralText
{
	// JSON Literal FText의 실제 문자열입니다.
	FString Text;
};

// CFMissileGuidePresetData의 P0 writable whole-record payload입니다.
struct FCFDAMissilePresetPayload
{
	// CFMissileGuidePresetData의 stable identity입니다.
	FName PresetId = NAME_None;

	// 사용자 표시 이름의 P0 Literal FText입니다.
	FCFDAStagingLiteralText PresetDisplayName;

	// 사용자 설명의 P0 Literal FText입니다.
	FCFDAStagingLiteralText PresetDescription;

	// 저장되는 raw authored MissileGuideConfig 전체입니다.
	FCFMissileGuideConfig MissileGuideConfig;
};

// strict JSON parse 뒤 authority/schema/payload를 모두 가진 typed Staging record입니다.
struct FCFDAStagingRecord
{
	// schema family identity입니다.
	FString SchemaId;

	// JSON shape revision입니다.
	int32 SchemaRevision = 0;

	// typed adapter semantic contract revision입니다.
	int32 AdapterContractRevision = 0;

	// exact supported Unreal DataAsset class path입니다.
	FString DataAssetTypeClassPath;

	// descriptor identity policy와 같은 FName stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// exact `/Game/.../Asset.Asset` target object path입니다.
	FString TargetObjectPath;

	// Update baseline이 존재하는지 나타냅니다. false이면 Create-intent record입니다.
	bool bHasBaseSemanticFingerprint = false;

	// Update record가 staging 작성 시점에 캡처한 persisted semantic baseline입니다.
	FString BaseSemanticFingerprint;

	// strict typed parse가 만든 MissileGuidePreset whole-record payload입니다.
	FCFDAMissilePresetPayload Payload;

	// 현재 typed payload에서 계산한 derived desired-state semantic fingerprint입니다.
	FString StagingSemanticFingerprint;

	// BatchPlanHash에 binding할 normalized main_game-relative Staging source path입니다.
	FString StagingRelativePath;
};

// strict JSON parse의 성공 여부, typed record와 진단을 함께 반환합니다.
struct FCFDAStagingParseResult
{
	// strict schema/value validation까지 모두 통과했는지 나타냅니다.
	bool bValid = false;

	// parse 성공 시 사용할 typed whole-record입니다.
	FCFDAStagingRecord Record;

	// parse/validation에서 수집한 stable diagnostic 목록입니다.
	TArray<FCFDAStagingIssue> Issues;
};

// Preview가 caller의 current-truth resolver로부터 받는 read-only target/identity 상태입니다.
struct FCFDAStagingCurrentState
{
	// requested exact TargetObjectPath에 현재 object가 존재하는지 나타냅니다.
	bool bRequestedTargetExists = false;

	// requested target package에 CF-FQ-049가 소유하지 않는 pre-existing dirty가 있는지 나타냅니다.
	bool bRequestedTargetDirty = false;

	// requested target이 존재할 때 확인한 exact concrete class path입니다.
	FString RequestedTargetClassPath;

	// requested target이 존재할 때 typed adapter로 읽은 stable identity입니다.
	FName RequestedTargetStableLogicalId = NAME_None;

	// requested target whole-record에서 동일 canonicalizer로 계산한 current semantic fingerprint입니다.
	FString CurrentSemanticFingerprint;

	// same stable logical identity가 current registry/asset set 안에 존재하는지 나타냅니다.
	bool bStableIdentityExists = false;

	// same stable logical identity와 일치한 current MissileGuidePreset asset 수입니다. 2개 이상이면 fail-closed duplicate identity입니다.
	int32 StableIdentityMatchCount = 0;

	// same stable logical identity가 정확히 1개 존재할 때의 canonical current object path입니다.
	FString StableIdentityObjectPath;
};

// 한 typed record와 current truth를 비교한 mutation0 Preview row입니다.
struct FCFDAStagingPreviewRow
{
	// Create/Update/NoChange/Conflict/Invalid 중 최종 Preview 분류입니다.
	ECFDAStagingPreviewKind Kind = ECFDAStagingPreviewKind::Invalid;

	// Preview 근거가 된 typed Staging record입니다.
	FCFDAStagingRecord Record;

	// target이 존재할 때 Preview 시점 current semantic fingerprint입니다.
	FString CurrentSemanticFingerprint;

	// blocking 또는 informational Preview diagnostic입니다.
	TArray<FCFDAStagingIssue> Issues;
};

// CF-FQ-049 P0-02 strict typed staging parser/canonicalizer/Preview pure service입니다.
class UCFMissileGuidePresetData;

class FCFDAStagingService
{
public:
	// P0 MissileGuidePreset schema identity를 반환합니다.
	static const TCHAR* GetMissilePresetSchemaId();

	// P0 MissileGuidePreset JSON shape revision을 반환합니다.
	static int32 GetMissilePresetSchemaRevision();

	// P0 MissileGuidePreset typed adapter semantic revision을 반환합니다.
	static int32 GetMissilePresetAdapterRevision();

	// P0 MissileGuidePreset exact native class path를 반환합니다.
	static const TCHAR* GetMissilePresetClassPath();

	// strict whole-record JSON을 typed MissileGuidePreset record로 parse/canonicalize하며 source path가 주어지면 canonical Staging root 아래 JSON인지 검증합니다.
	static FCFDAStagingParseResult ParseMissilePresetJson(
		const FString& JsonText,
		const FString& StagingRelativePath = FString());

	// typed MissileGuidePreset whole-record payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
	static bool BuildSemanticFingerprint(
		const FCFDAMissilePresetPayload& Payload,
		FString& OutFingerprint,
		FString& OutError);

	// exact MissileGuidePreset UObject를 source-backed P0 Literal whole-record semantic payload로 lossless 추출하며 UE package-only stable localization identity는 semantic metadata에서 제외합니다.
	static bool ExtractMissilePresetPayload(
		const UCFMissileGuidePresetData& PresetAsset,
		FCFDAMissilePresetPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Asset Registry와 typed MissileGuidePreset payload만 사용해 exact target/StableIdentity/current fingerprint를 read-only로 해석합니다.
	static bool ResolveMissilePresetCurrentState(
		const FCFDAStagingRecord& Record,
		FCFDAStagingCurrentState& OutCurrentState,
		TArray<FCFDAStagingIssue>& OutIssues);

	// valid typed record와 current truth를 P0 exact 3-way 규칙으로 mutation0 분류하며 mutable TargetObjectPath와 Payload↔cached fingerprint 불일치도 Invalid로 차단합니다.
	static FCFDAStagingPreviewRow BuildPreview(
		const FCFDAStagingRecord& Record,
		const FCFDAStagingCurrentState& CurrentState);

	// 같은 batch 안의 duplicate stable identity/target path를 fail-closed Invalid로 승격합니다.
	static void ApplyBatchDuplicateValidation(TArray<FCFDAStagingPreviewRow>& InOutRows);

	// exact Create/Update candidate set의 schema/identity/target/source path/intent/Payload↔fingerprint integrity를 재확인한 뒤 deterministic SHA-256 BatchPlanHash를 계산합니다.
	static bool BuildBatchPlanHash(
		const TArray<FCFDAStagingPreviewRow>& PreviewRows,
		FString& OutBatchPlanHash,
		FString& OutError);

	// diagnostic 배열에 특정 machine code가 존재하는지 확인합니다.
	static bool HasIssueCode(
		const TArray<FCFDAStagingIssue>& Issues,
		ECFDAStagingIssueCode Code);
};
