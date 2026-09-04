// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAManagementTypes.h
// Version: v1.5.0
// Date: 2026-09-03
// Description: CF-FQ-045 inventory DTO + Typed Semantic/Identity/Validation + Manager user semantics 결과 모델입니다.
// Changelog:
// - v1.5.0: Manager의 '어떤 데이터인가 / 어디에 사용되는가'용 user purpose/usage semantic metadata를 추가.
// - v1.4.0: loaded evaluation 상태, duplicate NotAnalyzed/N-A/Unique/Duplicate, typed canonical stable identity key를 추가.
// - v1.3.0: DAM-P0-02C용 InventoryGeneration, loaded identity/health/duplicate 결과와 freshness 계약을 추가.
// - v1.2.2: duplicate namespace derivation policy와 Unregistered policy-authority guard를 추가.
// - v1.2.1: Plan v0.1.4 계약에 맞춰 IdentityPolicy의 Required/Optional/N/A와 IdentityResolverKind를 분리.
// - v1.2.0: DAM-P0-02B용 Identity/Validation policy와 descriptor source metadata를 추가.
// - v1.1.0: 사용자용 Domain, semantic descriptor, registered/unregistered resolved semantic DTO를 추가.
// - v1.0.0: Registry readiness, Coverage, TypeInstance, Scope, Health/StableId unresolved 상태와 pure inventory input/output DTO를 추가.
// Migration:
// - 기존 Runtime DataAsset/Content 변경 없음. Policy는 선언 metadata일 뿐 Stable ID resolve나 Validation 실행을 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"

// Asset Registry metadata inventory의 준비 상태입니다.
enum class ECFDARegistryState : uint8
{
	Ready,
	Gathering,
	Failed
};

// 발견된 타입의 Typed Semantic descriptor coverage 상태입니다.
enum class ECFDACoverageState : uint8
{
	Registered,
	Unregistered
};

// 발견된 타입이 persisted instance를 가지고 있는지 나타냅니다.
enum class ECFDATypeInstanceState : uint8
{
	HasAssets,
	NoAssetInstance
};

// 사용자가 Data Overview에서 보는 큰 semantic Domain입니다.
enum class ECFDADomain : uint8
{
	Vehicle,
	Combat,
	Targeting,
	UI,
	Authoring,
	Unclassified
};

// Typed Semantic descriptor에서 stable identity가 필수인지 선언합니다.
enum class ECFDAIdentityPolicy : uint8
{
	Required,
	Optional,
	NotApplicable
};

// Stable identity가 존재하는 타입에서 실제 identity를 어떤 Source 계약으로 해석할지 선언합니다.
enum class ECFDAIdentityResolverKind : uint8
{
	None,
	ExplicitFName,
	ExplicitGuid,
	PrimaryAssetId
};

// Stable ID duplicate 비교 namespace를 어떤 기준으로 유도할지 선언합니다.
enum class ECFDADuplicateNamespacePolicy : uint8
{
	NotApplicable,
	ExactClassPath,
	PrimaryAssetType
};

// Typed Semantic descriptor가 loaded health lane에서 사용할 검증 계약 종류를 선언합니다.
enum class ECFDAValidationPolicy : uint8
{
	None,
	NativeDataValidation,
	CustomContract
};

// persisted DataAsset의 path-derived 관리 Scope입니다.
enum class ECFDAScope : uint8
{
	Test,
	Legacy,
	Debug,
	Authoring,
	RuntimeContent,
	Unclassified
};

// DAM-P0-01에서 아직 실행하지 않는 loaded Validation 상태를 표현합니다.
enum class ECFDAHealthState : uint8
{
	OK,
	Warning,
	Error,
	NotValidated,
	NotApplicable
};

// DAM-P0-01 metadata-only Refresh에서 Stable ID의 lazy resolve 상태를 표현합니다.
enum class ECFDAStableIdState : uint8
{
	Resolved,
	NotResolved,
	NotApplicable,
	MissingRequired
};

// explicit loaded lane 자체가 정상적으로 평가를 수행했는지 나타냅니다. DA Health와 운영/adapter 실패를 분리합니다.
enum class ECFDAEvaluationState : uint8
{
	NotRequested,
	Succeeded,
	PolicyUnavailable,
	InvalidRequest,
	LoadFailed,
	ClassMismatch,
	AdapterUnavailable
};

// Stable ID duplicate 분석이 수행됐는지와 최종 uniqueness 결과를 분리해 표현합니다.
enum class ECFDADuplicateState : uint8
{
	NotAnalyzed,
	NotApplicable,
	Unique,
	Duplicate
};

// duplicate equality에 사용할 stable identity 원본 값 종류입니다.
enum class ECFDAStableIdentityValueKind : uint8
{
	None,
	Name,
	Guid
};

// 표시 문자열과 분리된 typed/canonical duplicate identity key입니다.
struct FCFDACanonicalStableIdentity
{
	// duplicate namespace입니다. FName/Guid는 exact ClassPath, PrimaryAssetId는 PrimaryAssetType입니다.
	FString Namespace;

	// FName/PrimaryAssetName 또는 FGuid 중 어떤 값을 비교할지 나타냅니다.
	ECFDAStableIdentityValueKind ValueKind = ECFDAStableIdentityValueKind::None;

	// ExplicitFName 또는 PrimaryAssetId의 PrimaryAssetName 원본 값입니다.
	FName NameValue = NAME_None;

	// ExplicitGuid 원본 값입니다.
	FGuid GuidValue;

	// duplicate comparison에 사용할 수 있는 완전한 key인지 반환합니다.
	bool IsValid() const
	{
		if (Namespace.IsEmpty())
		{
			return false;
		}

		if (ValueKind == ECFDAStableIdentityValueKind::Name)
		{
			return !NameValue.IsNone();
		}

		if (ValueKind == ECFDAStableIdentityValueKind::Guid)
		{
			return GuidValue.IsValid();
		}

		return false;
	}

	// Unreal 원본 타입 equality를 보존한 duplicate equality입니다.
	bool Equals(const FCFDACanonicalStableIdentity& Other) const
	{
		if (Namespace != Other.Namespace || ValueKind != Other.ValueKind)
		{
			return false;
		}

		if (ValueKind == ECFDAStableIdentityValueKind::Name)
		{
			return NameValue == Other.NameValue;
		}

		if (ValueKind == ECFDAStableIdentityValueKind::Guid)
		{
			return GuidValue == Other.GuidValue;
		}

		return false;
	}
};

// Typed Registry가 class path 하나에 연결하는 최소 사용자 semantic descriptor입니다.
struct FCFDASemanticDescriptor
{
	// Unreal canonical class path입니다.
	FString ClassPath;

	// 사용자가 Overview에서 탐색할 큰 semantic Domain입니다.
	ECFDADomain Domain = ECFDADomain::Unclassified;

	// 기술 클래스명 대신 표시할 사용자용 Type 이름입니다.
	FString TypeDisplayName;

	// 사용자가 '어떤 데이터인가'에서 읽는 짧고 목적 중심의 설명입니다. Registry global required는 아닙니다.
	FString UserPurposeDescription;

	// 사용자가 '어디에 사용되는가'에서 읽는 실제 사용처 설명입니다. Registry global required는 아닙니다.
	FString UserUsageDescription;

	// 개발/진단 Detail에서 보존할 타입의 기술적 역할 설명입니다.
	FString RoleDescription;

	// Stable identity가 Required / Optional / N/A인지 나타냅니다. DAM-P0-02B에서는 resolve하지 않습니다.
	ECFDAIdentityPolicy IdentityPolicy = ECFDAIdentityPolicy::NotApplicable;

	// stable identity가 있는 타입의 실제 Source resolver 종류입니다.
	ECFDAIdentityResolverKind IdentityResolverKind = ECFDAIdentityResolverKind::None;

	// 실제 Source에서 확인한 identity field/function 이름입니다. IdentityPolicy=N/A이면 비어 있습니다.
	FString IdentitySourceName;

	// Loaded health lane에서 사용할 검증 계약 종류입니다. DAM-P0-02B에서는 실행하지 않습니다.
	ECFDAValidationPolicy ValidationPolicy = ECFDAValidationPolicy::None;

	// 실제 Source에서 확인한 validation entry/underlying contract 이름입니다. ValidationPolicy=None이면 비어 있습니다.
	FString ValidationSourceName;
};

// Registry lookup 결과와 fallback 여부를 함께 전달하는 semantic DTO입니다.
struct FCFDAResolvedSemantic
{
	// 등록 descriptor 또는 안전한 Unregistered fallback descriptor입니다.
	FCFDASemanticDescriptor Descriptor;

	// exact class path가 Typed Registry에 등록되어 있는지 나타냅니다.
	ECFDACoverageState CoverageState = ECFDACoverageState::Unregistered;

	// Identity/Validation policy를 authoritative하게 해석해도 되는 Registered semantic인지 반환합니다.
	bool HasAuthoritativePolicy() const
	{
		return CoverageState == ECFDACoverageState::Registered;
	}
};

// native CarFight class discovery가 pure analyzer에 전달하는 metadata입니다.
struct FCFDANativeTypeMetadata
{
	// Unreal canonical class path입니다.
	FString ClassPath;

	// 사용자 semantic mapping 전의 기술 클래스명입니다.
	FString TechnicalClassName;

	// native class가 abstract framework base인지 나타냅니다.
	bool bAbstract = false;
};

// Asset Registry가 pure analyzer에 전달하는 persisted Asset metadata입니다.
struct FCFDAPersistedAssetMetadata
{
	// Asset 이름입니다.
	FString AssetName;

	// Asset의 canonical object path입니다.
	FString ObjectPath;

	// Asset package path입니다.
	FString PackagePath;

	// Asset Registry가 보고한 concrete asset class path입니다.
	FString ClassPath;
};

// Generic Type inventory 한 행입니다.
struct FCFDATypeRecord
{
	// Unreal canonical class path입니다.
	FString ClassPath;

	// Typed descriptor가 없어도 표시 가능한 기술 클래스명입니다.
	FString TechnicalClassName;

	// native canonical CarFight class인지 나타냅니다.
	bool bCanonicalNative = false;

	// abstract framework base인지 나타냅니다.
	bool bAbstract = false;

	// 이 exact class path로 발견된 persisted Asset 수입니다.
	int32 AssetCount = 0;

	// Typed Semantic descriptor 등록 여부입니다.
	ECFDACoverageState CoverageState = ECFDACoverageState::Unregistered;

	// persisted instance 존재 여부입니다.
	ECFDATypeInstanceState TypeInstanceState = ECFDATypeInstanceState::NoAssetInstance;
};

// Generic Asset inventory 한 행입니다.
struct FCFDAAssetRecord
{
	// Asset 이름입니다.
	FString AssetName;

	// Asset의 canonical object path입니다.
	FString ObjectPath;

	// Asset package path입니다.
	FString PackagePath;

	// Asset Registry가 보고한 concrete asset class path입니다.
	FString ClassPath;

	// path-derived 관리 Scope입니다.
	ECFDAScope Scope = ECFDAScope::Unclassified;

	// Asset class의 Typed Semantic descriptor 등록 여부입니다.
	ECFDACoverageState CoverageState = ECFDACoverageState::Unregistered;

	// DAM-P0-01 Refresh에서는 loaded validation을 실행하지 않으므로 기본 NotValidated입니다.
	ECFDAHealthState HealthState = ECFDAHealthState::NotValidated;

	// DAM-P0-01 Refresh에서는 Stable ID를 resolve하지 않으므로 기본 NotResolved입니다.
	ECFDAStableIdState StableIdState = ECFDAStableIdState::NotResolved;

	// Lazy loaded lane에서만 채우는 Stable ID 문자열이며 P0-01에서는 비어 있습니다.
	FString StableId;
};

// Refresh가 현재 Typed Registry와 연결될 때 descriptor 존재 여부만 전달하는 옵션입니다.
struct FCFDARefreshOptions
{
	// descriptor가 존재하는 canonical class path 집합입니다.
	TSet<FString> RegisteredDescriptorClassPaths;
};

// metadata-only Inventory Core의 전체 결과입니다.
struct FCFDAInventoryResult
{
	// 이 metadata-only snapshot을 식별하는 session-local generation입니다. 0은 generation 미지정 legacy 호출입니다.
	uint64 InventoryGeneration = 0;

	// Asset Registry readiness 상태입니다.
	ECFDARegistryState RegistryState = ECFDARegistryState::Failed;

	// Registry failure 또는 gathering 진단 메시지입니다.
	FString RegistryMessage;

	// deterministic class path 순으로 정렬된 Type records입니다.
	TArray<FCFDATypeRecord> TypeRecords;

	// deterministic object path 순으로 정렬된 Asset records입니다.
	TArray<FCFDAAssetRecord> AssetRecords;

	// 현재 결과를 complete inventory로 취급해도 되는지 반환합니다.
	bool IsInventoryComplete() const
	{
		return RegistryState == ECFDARegistryState::Ready;
	}

	// 이 snapshot이 caller가 알고 있는 current generation과 같은지 반환합니다.
	bool IsCurrentGeneration(const uint64 CurrentInventoryGeneration) const
	{
		return InventoryGeneration != 0 && InventoryGeneration == CurrentInventoryGeneration;
	}
};

// explicit Validate/Detail loaded lane에서 Asset 한 건을 해석한 결과입니다.
struct FCFDALoadedAssetResult
{
	// 이 loaded 결과가 종속된 metadata inventory generation입니다.
	uint64 InventoryGeneration = 0;

	// loaded 요청 대상 canonical object path입니다.
	FString ObjectPath;

	// loaded 요청 대상 canonical class path입니다.
	FString ClassPath;

	// semantic descriptor coverage 상태입니다.
	ECFDACoverageState CoverageState = ECFDACoverageState::Unregistered;

	// explicit loaded lane 자체가 정상 평가됐는지 또는 policy/load/adapter 단계에서 중단됐는지 나타냅니다.
	ECFDAEvaluationState EvaluationState = ECFDAEvaluationState::NotRequested;

	// explicit loaded lane에서 실제 UObject load가 성공했는지 나타냅니다.
	bool bAssetLoaded = false;

	// stable identity resolve 상태입니다.
	ECFDAStableIdState StableIdState = ECFDAStableIdState::NotResolved;

	// resolved stable identity의 표시 문자열입니다.
	FString StableId;

	// 사용자 표시/진단용 duplicate namespace입니다.
	FString DuplicateNamespace;

	// 표시 문자열과 분리된 typed/canonical duplicate identity key입니다.
	FCFDACanonicalStableIdentity CanonicalStableIdentity;

	// 이 결과의 duplicate namespace 후보 전체가 같은 inventory snapshot 안에서 평가됐는지 나타냅니다.
	bool bDuplicateNamespaceCoverageComplete = false;

	// duplicate 분석 수행 여부와 uniqueness 결과입니다.
	ECFDADuplicateState DuplicateState = ECFDADuplicateState::NotAnalyzed;

	// identity + typed validation + duplicate 분석을 반영한 loaded Health입니다.
	ECFDAHealthState HealthState = ECFDAHealthState::NotValidated;

	// 사용자 Detail에서 표시할 bounded validation/identity 진단 메시지입니다.
	TArray<FString> Messages;

	// 이 loaded 결과를 current inventory generation에서 재사용해도 되는지 반환합니다.
	bool IsFreshForInventoryGeneration(const uint64 CurrentInventoryGeneration) const
	{
		return InventoryGeneration != 0 && InventoryGeneration == CurrentInventoryGeneration;
	}
};
