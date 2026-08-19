// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleResolverTypes.h
// Version: v1.1.0
// Date: 2026-08-17
// Description: DAUTH-P0-08E/F Pure Resolver의 immutable request/result/source-trace/definition-validation 계약입니다.
// Scope: Section 22.22~22.38 Resolver I/O, stage, source trace, measurement, diff, validation, stale value types를 제공합니다.
// Changelog:
// - v1.1.0: DAUTH-P0-08F UCFVDAValidator의 기존 문자열 FieldPath를 손실 없이 보존하는 ValidatorFieldPath 추가, R15 DefinitionValidation slot 설명 갱신.
// - v1.0.0: Frozen FCFVehicleResolveRequest/Result와 R0~R16 stage/output value contract 최초 구현.
// Migration:
// - 모든 타입은 CarFight_ReEditor Authoring 전용이며 Runtime UCFVehicleData schema를 변경하지 않습니다.
// - Apply/UI/CSV mutation 계약은 포함하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"
#include "CFVehicleResolverTypes.generated.h"

/** Pure Resolver의 최종 기술 상태입니다. */
UENUM()
enum class ECFVehicleResolveStatus : uint8
{
	Success,
	Blocked,
	Error
};

/** Frozen Section 22.24 R0~R16 실행 단계를 stable enum으로 고정합니다. */
UENUM()
enum class ECFVehicleResolverStage : uint8
{
	R0_RequestValidation,
	R1_ProjectDefaults,
	R2_Profiles,
	R3_RecipeSemantic,
	R4_DrivingFeel,
	R5_AssetSockets,
	R6_MeasurementProposal,
	R7_AcceptedMeasurement,
	R8_LegacyPins,
	R9_LegacySerialized,
	R10_AdvancedOverride,
	R11_DerivedGates,
	R12_CrossFieldValidation,
	R13_SourceTraceHash,
	R14_FieldDiff,
	R15_DefinitionMaterialization,
	R16_StaleDrift
};

/** Stage가 이번 Foundation에서 어떤 상태로 끝났는지 나타냅니다. */
UENUM()
enum class ECFVehicleResolverStageStatus : uint8
{
	Completed,
	Deferred,
	Failed
};

/** Resolver issue의 처리 수준입니다. */
UENUM()
enum class ECFVehicleValidationSeverity : uint8
{
	Info,
	Warning,
	Blocked,
	Error
};

/** Frozen stage order가 실제 호출 순서대로 실행됐는지 기록하는 compact record입니다. */
USTRUCT()
struct FCFVehicleResolverStageRecord
{
	GENERATED_BODY()

	// 실행한 Frozen R0~R16 stage입니다.
	UPROPERTY()
	ECFVehicleResolverStage Stage = ECFVehicleResolverStage::R0_RequestValidation;

	// 해당 stage의 Foundation 실행 상태입니다.
	UPROPERTY()
	ECFVehicleResolverStageStatus Status = ECFVehicleResolverStageStatus::Completed;
};

/** Preview/Fitting이 제공할 수 있지만 Source winner가 될 수 없는 보조 context입니다. */
USTRUCT()
struct FCFVehicleResolverPreviewContext
{
	GENERATED_BODY()

	// Preview UI가 참고할 fitted total mass가 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasFittedTotalMassKg = false;

	// Definition 생성 Source가 아닌 Preview 전용 fitted total mass kg입니다.
	UPROPERTY()
	float FittedTotalMassKg = 0.0f;
};

/** 하나의 field candidate가 어떤 Source에서 왔는지 설명하는 preview layer입니다. */
USTRUCT()
struct FCFVehicleSourceLayer
{
	GENERATED_BODY()

	// Frozen Authoring Source 종류입니다.
	UPROPERTY()
	ECFVehicleSourceType SourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;

	// Profile path, Recipe identity, Asset path, Rule identity 등 source identity입니다.
	UPROPERTY()
	FString SourceId;

	// 사람이 읽는 diagnostic revision이며 signature authority 단독 근거가 아닙니다.
	UPROPERTY()
	int32 SourceRevision = 0;

	// 이 layer가 실제 소비한 semantic payload fingerprint입니다.
	UPROPERTY()
	FString SourceFingerprint;

	// 이 layer value의 deterministic value hash입니다.
	UPROPERTY()
	FString ValueHash;

	// Stable path/rule/dependency/contract까지 결합한 field-level source signature입니다.
	UPROPERTY()
	FString SourceSignature;

	// 현재 stack에서 이 layer가 effective winner인지 여부입니다.
	UPROPERTY()
	bool bEffective = false;
};

/** Field별 candidate stack과 effective/shadow signature를 보존하는 Preview Source Trace입니다. */
USTRUCT()
struct FCFVehicleSourceTrace
{
	GENERATED_BODY()

	// Trace가 설명하는 exact stable field path입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// 낮은 precedence에서 높은 precedence 순으로 정렬된 source layer입니다.
	UPROPERTY()
	TArray<FCFVehicleSourceLayer> Layers;

	// Layers 안의 effective winner index입니다.
	UPROPERTY()
	int32 EffectiveLayerIndex = INDEX_NONE;

	// Effective winner의 deterministic source signature입니다.
	UPROPERTY()
	FString EffectiveSourceSignature;

	// Effective 아래 shadow layer signature를 canonical order로 결합한 hash입니다.
	UPROPERTY()
	FString ShadowSourceSignature;
};

/** Resolver가 최종 effective winner로 선택한 exact leaf value입니다. */
USTRUCT()
struct FCFVehicleResolvedField
{
	GENERATED_BODY()

	// Scalar 또는 stable selector가 확정된 exact field path입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// Effective winner의 canonical typed field value입니다.
	UPROPERTY()
	FCFVehicleFieldValue Value;

	// PreviewSourceTrace에서 이 field trace를 찾을 index입니다.
	UPROPERTY()
	int32 SourceTraceIndex = INDEX_NONE;
};

/** Adoption 전 effective value를 바꾸지 않는 Wheel measurement candidate입니다. */
USTRUCT()
struct FCFVehicleMeasurementProposal
{
	GENERATED_BODY()

	// Proposal이 대상으로 하는 exact Definition field입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// Wheel bounds에서 계산한 typed candidate value입니다.
	UPROPERTY()
	FCFVehicleFieldValue MeasuredCandidateValue;

	// Proposal 계산에 사용한 Wheel resolver-relevant asset fingerprint입니다.
	UPROPERTY()
	FString AssetFingerprint;

	// Measurement 수학을 식별하는 stable rule ID입니다.
	UPROPERTY()
	FName MeasurementRuleId = NAME_None;
};

/** Recipe/Resolver/Definition validation layer에서 공유하는 구조화 issue입니다. */
USTRUCT()
struct FCFVehicleValidationIssue
{
	GENERATED_BODY()

	// 이 issue가 resolve를 막는 정도입니다.
	UPROPERTY()
	ECFVehicleValidationSeverity Severity = ECFVehicleValidationSeverity::Info;

	// 자동화/AI가 문자열 번역 없이 분류할 stable issue code입니다.
	UPROPERTY()
	FName IssueCode = NAME_None;

		// Field-specific Authoring issue일 때 사용할 exact stable path입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// 기존 UCFVDAValidator처럼 index/aggregate 문자열 path를 반환하는 검증기의 원본 FieldPath입니다.
	UPROPERTY()
	FString ValidatorFieldPath;

	// 사람이 읽는 한국어 진단 메시지입니다.
	UPROPERTY()
	FString Message;
};

/** R14 Foundation에서 표현하는 deterministic field diff row입니다. */
USTRUCT()
struct FCFVehicleFieldDiff
{
	GENERATED_BODY()

	// Set/Add/Remove/Move 중 diff operation입니다.
	UPROPERTY()
	ECFVehicleDiffOp Operation = ECFVehicleDiffOp::SetLeaf;

	// Diff 대상 stable field path입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// Current Definition에 값이 실제 존재했는지 여부입니다.
	UPROPERTY()
	bool bHasBeforeValue = false;

	// Current Definition의 canonical before value입니다.
	UPROPERTY()
	FCFVehicleFieldValue BeforeValue;

	// Resolved candidate에 값이 실제 존재했는지 여부입니다.
	UPROPERTY()
	bool bHasAfterValue = false;

	// Resolver의 canonical after value입니다.
	UPROPERTY()
	FCFVehicleFieldValue AfterValue;

	// AfterValue를 설명하는 PreviewSourceTrace index입니다.
	UPROPERTY()
	int32 SourceTraceIndex = INDEX_NONE;
};

/** Applied baseline과 현재 resolve/target 차이를 field 단위로 설명하는 stale row입니다. */
USTRUCT()
struct FCFVehicleStaleField
{
	GENERATED_BODY()

	// Stale/Drift가 관측된 exact stable field path입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// Effective source signature가 last apply와 달라졌는지 여부입니다.
	UPROPERTY()
	bool bEffectiveSourceChanged = false;

	// Effective winner 아래 shadow source stack만 달라졌는지 여부입니다.
	UPROPERTY()
	bool bShadowSourceChanged = false;

	// Current Target value가 last applied field value와 달라졌는지 여부입니다.
	UPROPERTY()
	bool bExternalDrift = false;

	// 마지막 Apply effective source signature입니다.
	UPROPERTY()
	FString LastAppliedSourceSignature;

	// 현재 Resolve effective source signature입니다.
	UPROPERTY()
	FString CurrentSourceSignature;

	// 마지막 Apply exact value hash입니다.
	UPROPERTY()
	FString LastAppliedValueHash;

	// 현재 Target exact value hash입니다.
	UPROPERTY()
	FString CurrentTargetValueHash;
};

/** Section 22.29의 aggregate stale/drift report입니다. */
USTRUCT()
struct FCFVehicleStaleReport
{
	GENERATED_BODY()

	// Effective Source change가 하나 이상 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasEffectiveStale = false;

	// Shadow Source change가 하나 이상 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasShadowSourceChange = false;

	// Current Target external drift가 하나 이상 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasExternalDrift = false;

	// Canonical field path 순으로 정렬된 stale rows입니다.
	UPROPERTY()
	TArray<FCFVehicleStaleField> Fields;
};

/** Pure Resolver가 live UObject/Slate를 재조회하지 않고 소비하는 immutable-style 요청입니다. */
USTRUCT()
struct FCFVehicleResolveRequest
{
	GENERATED_BODY()

	// Persistent Recipe에서 복사된 semantic authoring snapshot입니다.
	UPROPERTY()
	FCFVehicleRecipeSnapshot Recipe;

	// Frozen 5개 Profile typed snapshot set입니다.
	UPROPERTY()
	FCFVehicleProfileSnapshotSet Profiles;

	// Current UCFVehicleData C++ defaults에서 만든 117-pattern compatibility baseline입니다.
	UPROPERTY()
	FCFVehicleDefinitionSnapshot ProjectDefaults;

	// Chassis Socket / Wheel Bounds를 value-copy한 asset snapshot입니다.
	UPROPERTY()
	FCFVehicleAssetSnapshot Assets;

	// Diff/Drift용 Current Definition snapshot이 포함됐는지 여부입니다.
	UPROPERTY()
	bool bHasCurrentDefinition = false;

	// Optional Current Target Definition exact snapshot입니다.
	UPROPERTY()
	FCFVehicleDefinitionSnapshot CurrentDefinition;

	// Preview-only Fitting context가 포함됐는지 여부입니다.
	UPROPERTY()
	bool bHasPreviewContext = false;

	// Source winner가 될 수 없는 optional Fitting preview context입니다.
	UPROPERTY()
	FCFVehicleResolverPreviewContext PreviewContext;

	// 요청자가 기대하는 Frozen Resolver semantic contract revision입니다.
	UPROPERTY()
	int32 ResolverContractRevision = 1;
};

/** Section 22.23 Pure Resolver의 deterministic value-copy 결과입니다. */
USTRUCT()
struct FCFVehicleResolveResult
{
	GENERATED_BODY()

	// Validation/conflict를 반영한 최종 Resolve status입니다.
	UPROPERTY()
	ECFVehicleResolveStatus ResolveStatus = ECFVehicleResolveStatus::Error;

	// Frozen R0~R16 실제 실행 순서와 Foundation deferred stage를 기록합니다.
	UPROPERTY()
	TArray<FCFVehicleResolverStageRecord> StageRecords;

	// Canonical stable field path 오름차순 effective leaf set입니다.
	UPROPERTY()
	TArray<FCFVehicleResolvedField> SortedResolvedFields;

	// SortedResolvedFields와 같은 canonical field 순서의 source stack입니다.
	UPROPERTY()
	TArray<FCFVehicleSourceTrace> PreviewSourceTrace;

	// Adoption 전에는 effective field를 바꾸지 않는 Wheel measurement candidate입니다.
	UPROPERTY()
	TArray<FCFVehicleMeasurementProposal> MeasurementProposals;

	// Current Definition이 있을 때 생성하는 deterministic leaf diff foundation입니다.
	UPROPERTY()
	TArray<FCFVehicleFieldDiff> FieldDiff;

	// R0 Recipe/Binding validation issues입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> RecipeValidation;

	// Candidate conflict/source/mass/socket/cross-field validation issues입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> ResolverValidation;

		// R15 transient candidate에 기존 UCFVDAValidator를 실행해 만든 Definition validation issues입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> DefinitionValidation;

	// AppliedState와 optional CurrentDefinition을 비교한 Foundation stale/drift report입니다.
	UPROPERTY()
	FCFVehicleStaleReport StaleReport;

	// 모든 effective field source signature를 canonical order로 결합한 deterministic hash입니다.
	UPROPERTY()
	FString SourceSignature;

	// SortedResolvedFields path/type/value를 canonical order로 hash한 deterministic definition hash입니다.
	UPROPERTY()
	FString ResolvedDefinitionHash;

	// 실제 Resolve에 사용된 semantic contract revision입니다.
	UPROPERTY()
	int32 ResolverContractRevision = 0;
};
