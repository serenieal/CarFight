// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRefEvidence.h
// Version: v1.0.0
// Date: 2026-08-26
// Description: CF-FQ-040 VB-P0-01/05 Vehicle Reference Evidence Editor-only DataAsset와 typed stale-binding schema입니다.
// Scope: Reference identity/source/claim/conflict/unknown fact와 deterministic SHA-256 Evidence fingerprint를 소유합니다.
// Changelog:
// - v1.0.0: VB-P0-05 설계 검수 교정으로 VB-P0-01 frozen schema를 실제 Editor-only DataAsset contract로 구현.
// Migration:
// - Runtime UCFVehicleData에는 citation/provenance를 추가하지 않습니다.
// - EvidenceFingerprint는 semantic payload에서 생성되는 read-only 값이며 caller가 임의 문자열을 authority로 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFVehicleRefEvidence.generated.h"

/** Reference Vehicle이 Builder에서 수행하는 역할입니다. */
UENUM(BlueprintType)
enum class ECFRefVehicleRole : uint8
{
	Primary,
	Secondary,
	TraitOnly
};

/** Model year 표현 방식입니다. */
UENUM(BlueprintType)
enum class ECFRefModelYearQualifier : uint8
{
	Exact,
	Range,
	CurrentAsObserved,
	Unknown
};

/** Evidence source 신뢰도 tier입니다. */
UENUM(BlueprintType)
enum class ECFRefSourceTier : uint8
{
	TierA,
	TierB,
	TierC,
	TierD
};

/** 서로 다른 citation이 원 출처까지 독립적인지 나타냅니다. */
UENUM(BlueprintType)
enum class ECFRefOriginIndependence : uint8
{
	SameOrigin,
	IndependentOrigin,
	OriginUnknown
};

/** Atomic claim 값 표현 방식입니다. */
UENUM(BlueprintType)
enum class ECFRefValueKind : uint8
{
	Number,
	Integer,
	Boolean,
	CanonicalText,
	Range
};

/** Claim의 사실/계산/게임 조정 provenance입니다. */
UENUM(BlueprintType)
enum class ECFRefProvenance : uint8
{
	FACT,
	DERIVED,
	GAME_BIAS
};

/** Claim이 canonical evidence로 채택됐는지 나타냅니다. */
UENUM(BlueprintType)
enum class ECFRefClaimResolution : uint8
{
	Canonical,
	Candidate,
	Rejected
};

/** Source 간 충돌 유형입니다. */
UENUM(BlueprintType)
enum class ECFRefConflictType : uint8
{
	IdentityMismatch,
	VariantScopeMismatch,
	SourceDisagreement,
	UnitSemanticMismatch,
	MeasurementSpread,
	OutlierCandidate
};

/** Evidence conflict가 proposal을 막는 수준입니다. */
UENUM(BlueprintType)
enum class ECFRefConflictSeverity : uint8
{
	Info,
	Warning,
	Block
};

/** Evidence conflict 해결 정책입니다. */
UENUM(BlueprintType)
enum class ECFRefResolutionPolicy : uint8
{
	SplitIdentity,
	SplitVariant,
	PreferExactIdentity,
	PreferHigherTier,
	UseDerivedRange,
	RejectOutlier,
	UnresolvedBlock
};

/** 찾지 못한 fact의 사유입니다. */
UENUM(BlueprintType)
enum class ECFRefUnknownReason : uint8
{
	NotPublished,
	IdentityAmbiguous,
	ConflictingUnresolved,
	OutOfScope
};

/** Unknown fact가 후속 proposal에 미치는 영향입니다. */
UENUM(BlueprintType)
enum class ECFRefUnknownBlockingUse : uint8
{
	None,
	ProposalWarning,
	ProposalBlock
};

/** 하나의 정확한 Reference Vehicle identity입니다. */
USTRUCT(BlueprintType)
struct FCFRefVehicleIdentity
{
	GENERATED_BODY()

	// Evidence 안에서 Reference Vehicle을 안정적으로 식별하는 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference", meta=(ToolTip="이 Evidence 안에서 Reference Vehicle을 안정적으로 식별하는 ID입니다."))
	FName ReferenceVehicleId = NAME_None;

	// Primary/Secondary/TraitOnly 역할입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference", meta=(ToolTip="이 Reference Vehicle이 Builder에서 수행하는 역할입니다."))
	ECFRefVehicleRole Role = ECFRefVehicleRole::Primary;

	// 제조사 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString Manufacturer;

	// 모델 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString Model;

	// 세대/플랫폼 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString Generation;

	// 적용 모델 연도 시작입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	int32 ModelYearStart = 0;

	// 적용 모델 연도 끝입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	int32 ModelYearEnd = 0;

	// 연도 값이 Exact/Range/Current/Unknown 중 무엇인지 나타냅니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	ECFRefModelYearQualifier ModelYearQualifier = ECFRefModelYearQualifier::Unknown;

	// 트림 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString Trim;

	// 엔진/모터 등 파워트레인 식별자입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString Powertrain;

	// 변속기 identity 설명입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString Transmission;

	// FF/RWD/AWD 등 구동 layout 설명입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString DriveLayout;

	// KR/US/EU 등 시장 범위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString MarketRegion;

	// 차체 variant입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString BodyVariant;

	// 휠/타이어 variant입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference")
	FString WheelTireVariant;

	// Reference identity 자체의 confidence입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Reference", meta=(ClampMin="0.0", ClampMax="0.99"))
	float IdentityConfidence = 0.0f;
};

/** Reference fact를 뒷받침하는 citation 하나입니다. */
USTRUCT(BlueprintType)
struct FCFRefSourceCitation
{
	GENERATED_BODY()

	// Evidence 내부 stable source ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FName SourceId = NAME_None;

	// Source reliability tier입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	ECFRefSourceTier Tier = ECFRefSourceTier::TierD;

	// Manufacturer page/manual/database 등 source 종류입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FName SourceKind = NAME_None;

	// 게시자입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FString Publisher;

	// 문서/페이지 제목입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FString DocumentTitle;

	// tracking/fragment가 제거된 canonical URL입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FString CanonicalUrl;

	// 게시일 ISO 문자열입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FString PublishedDate;

	// 수정일 ISO 문자열입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FString UpdatedDate;

	// 표/섹션/페이지 등 fact를 찾은 위치입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FString Locator;

	// 이 source가 직접 다루는 Reference Vehicle ID들입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	TArray<FName> ReferenceVehicleIds;

	// 동일 원자료 여부를 판정하기 위한 origin group ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	FName OriginGroupId = NAME_None;

	// 서로 다른 publisher가 실제 원자료까지 독립적인지 나타냅니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Source")
	ECFRefOriginIndependence OriginIndependence = ECFRefOriginIndependence::OriginUnknown;

	// 조사 시각이며 fingerprint에서는 제외합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic")
	FDateTime AccessedAtUtc;

	// 사람이 읽는 source 범위 메모이며 fingerprint에서는 제외합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic", meta=(MultiLine="true"))
	FString SourceScopeNote;
};

/** 하나의 atomic normalized vehicle fact/derived/game-bias claim입니다. */
USTRUCT(BlueprintType)
struct FCFRefClaim
{
	GENERATED_BODY()

	// Evidence 내부 stable claim ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FName ClaimId = NAME_None;

	// 이 claim이 속하는 Reference Vehicle ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FName ReferenceVehicleId = NAME_None;

	// Runtime field path가 아닌 research semantic fact key입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FName FactKey = NAME_None;

	// claim 값의 표현 종류입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	ECFRefValueKind ValueKind = ECFRefValueKind::Number;

	// Number 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	double NumberValue = 0.0;

	// Integer 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	int64 IntegerValue = 0;

	// Boolean 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	bool BooleanValue = false;

	// CanonicalText 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FString TextValue;

	// Range 최소값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	double RangeMinValue = 0.0;

	// Range 최대값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	double RangeMaxValue = 0.0;

	// normalized unit ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FName UnitId = NAME_None;

	// 사람이 원문 수치를 확인할 수 있는 짧은 source value입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FString SourceValueText;

	// FACT/DERIVED/GAME_BIAS provenance입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	ECFRefProvenance Provenance = ECFRefProvenance::FACT;

	// FACT를 뒷받침하는 source citation ID들입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	TArray<FName> CitationIds;

	// DERIVED/GAME_BIAS의 입력 claim ID들입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	TArray<FName> InputClaimIds;

	// DERIVED/GAME_BIAS 계산 방법 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FName MethodId = NAME_None;

	// 계산 방법 revision입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	int32 MethodRevision = 0;

	// 계산 방법의 deterministic parameter key/value입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	TMap<FName, FString> MethodParameters;

	// source support confidence입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim", meta=(ClampMin="0.0", ClampMax="0.99"))
	float ConfidenceScore = 0.0f;

	// 연결된 conflict ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	FName ConflictId = NAME_None;

	// Canonical/Candidate/Rejected 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Claim")
	ECFRefClaimResolution ResolutionState = ECFRefClaimResolution::Candidate;
};

/** 동일 fact의 identity/variant/source 충돌 기록입니다. */
USTRUCT(BlueprintType)
struct FCFRefConflict
{
	GENERATED_BODY()

	// Evidence 내부 stable conflict ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	FName ConflictId = NAME_None;

	// 충돌 대상 Reference Vehicle ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	FName ReferenceVehicleId = NAME_None;

	// 충돌 대상 research semantic fact key입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	FName FactKey = NAME_None;

	// 충돌하는 candidate claim ID들입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	TArray<FName> CandidateClaimIds;

	// 충돌 유형입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	ECFRefConflictType ConflictType = ECFRefConflictType::SourceDisagreement;

	// Info/Warning/Block 수준입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	ECFRefConflictSeverity Severity = ECFRefConflictSeverity::Warning;

	// 해결 정책입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	ECFRefResolutionPolicy ResolutionPolicy = ECFRefResolutionPolicy::UnresolvedBlock;

	// 해결된 canonical claim ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	FName ResolutionClaimId = NAME_None;

	// machine-readable resolution reason입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Conflict")
	FName ResolutionReasonCode = NAME_None;

	// 사람이 읽는 보조 설명입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic", meta=(MultiLine="true"))
	FString Note;
};

/** 조사했지만 canonical value를 확정하지 못한 fact입니다. */
USTRUCT(BlueprintType)
struct FCFRefUnknownFact
{
	GENERATED_BODY()

	// Evidence 내부 stable unknown ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Unknown")
	FName UnknownFactId = NAME_None;

	// 대상 Reference Vehicle ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Unknown")
	FName ReferenceVehicleId = NAME_None;

	// 찾지 못한 research semantic fact key입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Unknown")
	FName FactKey = NAME_None;

	// 실제로 확인한 source ID들입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Unknown")
	TArray<FName> SearchedSourceIds;

	// 값을 확정하지 못한 이유입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Unknown")
	ECFRefUnknownReason Reason = ECFRefUnknownReason::NotPublished;

	// 이 unknown이 proposal에 미치는 영향입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Unknown")
	ECFRefUnknownBlockingUse BlockingUse = ECFRefUnknownBlockingUse::None;

	// 사람이 읽는 보조 설명입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic", meta=(MultiLine="true"))
	FString Note;
};

/** 한 Builder 차량의 Reference research companion SSOT입니다. */
UCLASS(BlueprintType)
class CARFIGHT_REEDITOR_API UCFVehicleRefEvidence : public UDataAsset
{
	GENERATED_BODY()

public:
	// 새 Evidence identity와 current schema/policy revision을 초기화합니다.
	UCFVehicleRefEvidence();

	// Reference Evidence는 Editor-only companion asset입니다.
	virtual bool IsEditorOnly() const override { return true; }

	// Duplicate가 원본 EvidenceId를 재사용하지 않도록 새 identity를 발급합니다.
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

#if WITH_EDITOR
	// Editor에서 semantic field가 바뀌면 generated fingerprint를 즉시 다시 계산합니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// 현재 semantic payload로 deterministic SHA-256 fingerprint를 계산합니다.
	bool BuildEvidenceFingerprint(FString& OutFingerprint, FString& OutError) const;

	// generated read-only EvidenceFingerprint를 현재 semantic payload로 갱신합니다.
	bool RefreshEvidenceFingerprint(FString& OutError);

	// 이 Evidence의 persistent identity입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Identity", meta=(ToolTip="이 Reference Evidence asset의 persistent identity입니다."))
	FGuid EvidenceId;

	// Evidence schema revision입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Identity")
	int32 SchemaRevision = 1;

	// Research normalization policy revision입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Identity")
	FString NormalizationPolicyRevision = TEXT("CFVRN-1");

	// 이 Evidence가 소유되는 Recipe identity입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Binding")
	FGuid TargetRecipeId;

	// 대상 Recipe soft path입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Binding")
	FSoftObjectPath TargetRecipePath;

	// 대상 UCFVehicleData soft path입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Binding")
	FSoftObjectPath TargetDefinitionPath;

	// 서로 섞으면 안 되는 exact Reference Vehicle identity 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Research")
	TArray<FCFRefVehicleIdentity> ReferenceVehicles;

	// Research citation 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Research")
	TArray<FCFRefSourceCitation> Sources;

	// FACT/DERIVED/GAME_BIAS atomic claim 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Research")
	TArray<FCFRefClaim> Claims;

	// Source/identity/variant conflict 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Research")
	TArray<FCFRefConflict> Conflicts;

	// 조사했지만 확정하지 못한 fact 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Research")
	TArray<FCFRefUnknownFact> UnknownFacts;

	// 현재 semantic payload에서 생성된 SHA-256 fingerprint입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Fingerprint", meta=(ToolTip="현재 Reference Evidence semantic payload에서 생성된 SHA-256 fingerprint입니다. 직접 입력하지 않습니다."))
	FString EvidenceFingerprint;

	// 사람이 읽는 diagnostic sequence이며 fingerprint에서는 제외합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic")
	int32 AuthoringRevision = 0;

	// 마지막 Research 수행 시각이며 fingerprint에서는 제외합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic")
	FDateTime LastResearchAtUtc;

	// 사람이 읽는 Research 메모이며 fingerprint에서는 제외합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Builder|Diagnostic", meta=(MultiLine="true"))
	FString ResearchNotes;
};
