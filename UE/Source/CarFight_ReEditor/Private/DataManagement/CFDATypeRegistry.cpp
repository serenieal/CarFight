// Copyright (c) CarFight. All Rights Reserved.
// File: CFDATypeRegistry.cpp
// Version: v1.2.0
// Date: 2026-09-03
// Description: CF-FQ-045 Typed Semantic Registry foundation + user/policy metadata 구현입니다.
// Changelog:
// - v1.2.0: optional user purpose/usage fallback과 descriptor equivalence를 추가. global required 규칙은 만들지 않음.
// - v1.1.2: identity resolver kind에서 duplicate namespace policy를 deterministic하게 유도하는 pure helper를 추가.
// - v1.1.1: Identity Required/Optional/N/A와 resolver kind 일관성 검사를 분리하고 equivalence에 resolver kind를 추가.
// - v1.1.0: Identity/Validation policy source metadata 완결성 검사와 descriptor equivalence 비교를 추가.
// - v1.0.0: exact-path 등록, conflict fail-closed, deterministic descriptor enumeration, Coverage bridge, Unregistered fallback을 구현.
// Migration:
// - UObject load, Stable ID resolve, Validation 실행, Duplicate ID, Reference query, UI mutation을 수행하지 않습니다.

#include "DataManagement/CFDATypeRegistry.h"

// descriptor를 exact class path 기준으로 등록합니다. 동일 descriptor 재등록은 idempotent하게 허용합니다.
bool FCFDATypeRegistry::RegisterDescriptor(
	const FCFDASemanticDescriptor& Descriptor,
	FString* OutError)
{
	if (OutError)
	{
		OutError->Reset();
	}

	if (Descriptor.ClassPath.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("Semantic descriptor ClassPath가 비어 있습니다.");
		}
		return false;
	}

	if (Descriptor.TypeDisplayName.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("Semantic descriptor TypeDisplayName이 비어 있습니다.");
		}
		return false;
	}

	if (Descriptor.RoleDescription.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("Semantic descriptor RoleDescription이 비어 있습니다.");
		}
		return false;
	}

	if (Descriptor.IdentityPolicy == ECFDAIdentityPolicy::NotApplicable)
	{
		if (Descriptor.IdentityResolverKind != ECFDAIdentityResolverKind::None || !Descriptor.IdentitySourceName.IsEmpty())
		{
			if (OutError)
			{
				*OutError = TEXT("IdentityPolicy=N/A인 descriptor는 identity resolver/source를 가질 수 없습니다.");
			}
			return false;
		}
	}
	else if (Descriptor.IdentityResolverKind == ECFDAIdentityResolverKind::None || Descriptor.IdentitySourceName.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("IdentityPolicy가 Required/Optional인 descriptor에는 resolver kind와 실제 Source identity 이름이 필요합니다.");
		}
		return false;
	}

	if (Descriptor.ValidationPolicy == ECFDAValidationPolicy::None && !Descriptor.ValidationSourceName.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("ValidationPolicy=None인 descriptor는 ValidationSourceName을 가질 수 없습니다.");
		}
		return false;
	}

	if (Descriptor.ValidationPolicy != ECFDAValidationPolicy::None && Descriptor.ValidationSourceName.IsEmpty())
	{
		if (OutError)
		{
			*OutError = TEXT("ValidationPolicy가 있는 descriptor에는 실제 Source validation 이름이 필요합니다.");
		}
		return false;
	}

	// 같은 class path에 이미 등록된 descriptor입니다.
	const FCFDASemanticDescriptor* ExistingDescriptor = DescriptorsByClassPath.Find(Descriptor.ClassPath);
	if (ExistingDescriptor)
	{
		if (AreDescriptorsEquivalent(*ExistingDescriptor, Descriptor))
		{
			return true;
		}

		if (OutError)
		{
			*OutError = FString::Printf(
				TEXT("Semantic descriptor conflict: %s"),
				*Descriptor.ClassPath);
		}
		return false;
	}

	DescriptorsByClassPath.Add(Descriptor.ClassPath, Descriptor);
	return true;
}

// exact class path에 등록된 descriptor를 찾습니다.
const FCFDASemanticDescriptor* FCFDATypeRegistry::FindDescriptor(const FString& ClassPath) const
{
	return DescriptorsByClassPath.Find(ClassPath);
}

// 등록 여부와 안전한 Unregistered fallback을 함께 반환합니다.
FCFDAResolvedSemantic FCFDATypeRegistry::ResolveSemantic(
	const FString& ClassPath,
	const FString& TechnicalClassName) const
{
	// caller에게 반환할 semantic lookup 결과입니다.
	FCFDAResolvedSemantic Result;

	// exact class path에 등록된 descriptor입니다.
	const FCFDASemanticDescriptor* RegisteredDescriptor = FindDescriptor(ClassPath);
	if (RegisteredDescriptor)
	{
		Result.Descriptor = *RegisteredDescriptor;
		Result.CoverageState = ECFDACoverageState::Registered;
		return Result;
	}

	Result.Descriptor.ClassPath = ClassPath;
	Result.Descriptor.Domain = ECFDADomain::Unclassified;
	Result.Descriptor.TypeDisplayName = TechnicalClassName.IsEmpty()
		? ClassPath
		: TechnicalClassName;
	Result.Descriptor.UserPurposeDescription =
		TEXT("관리 설명이 아직 등록되지 않은 DataAsset 타입입니다.");
	Result.Descriptor.UserUsageDescription =
		TEXT("현재는 발견된 타입과 Asset 위치를 기준으로 확인할 수 있으며, 관리 규칙이 등록되면 구체적인 사용처가 표시됩니다.");
	Result.Descriptor.RoleDescription =
		TEXT("Typed Semantic descriptor가 아직 등록되지 않은 DataAsset 타입입니다.");
	Result.CoverageState = ECFDACoverageState::Unregistered;
	return Result;
}

// descriptor의 identity 계약에서 duplicate 비교 namespace 정책을 deterministic하게 유도합니다.
ECFDADuplicateNamespacePolicy FCFDATypeRegistry::ResolveDuplicateNamespacePolicy(
	const FCFDASemanticDescriptor& Descriptor)
{
	if (Descriptor.IdentityPolicy == ECFDAIdentityPolicy::NotApplicable)
	{
		return ECFDADuplicateNamespacePolicy::NotApplicable;
	}

	switch (Descriptor.IdentityResolverKind)
	{
	case ECFDAIdentityResolverKind::ExplicitFName:
	case ECFDAIdentityResolverKind::ExplicitGuid:
		return ECFDADuplicateNamespacePolicy::ExactClassPath;
	case ECFDAIdentityResolverKind::PrimaryAssetId:
		return ECFDADuplicateNamespacePolicy::PrimaryAssetType;
	default:
		return ECFDADuplicateNamespacePolicy::NotApplicable;
	}
}

// 현재 Registry coverage를 P0-01 metadata Refresh 옵션으로 변환합니다.
FCFDARefreshOptions FCFDATypeRegistry::BuildRefreshOptions() const
{
	// P0-01 metadata inventory에 전달할 descriptor coverage 옵션입니다.
	FCFDARefreshOptions Options;
	Options.RegisteredDescriptorClassPaths.Reserve(DescriptorsByClassPath.Num());

	// Registry의 class path identity만 Coverage seam으로 전달합니다.
	for (const TPair<FString, FCFDASemanticDescriptor>& DescriptorPair : DescriptorsByClassPath)
	{
		Options.RegisteredDescriptorClassPaths.Add(DescriptorPair.Key);
	}

	return Options;
}

// 등록 descriptor를 canonical class path 순으로 반환합니다.
TArray<FCFDASemanticDescriptor> FCFDATypeRegistry::GetDescriptors() const
{
	// deterministic enumeration을 위한 descriptor 배열입니다.
	TArray<FCFDASemanticDescriptor> Descriptors;
	DescriptorsByClassPath.GenerateValueArray(Descriptors);

	Descriptors.Sort([](
		const FCFDASemanticDescriptor& Left,
		const FCFDASemanticDescriptor& Right)
	{
		return Left.ClassPath.Compare(Right.ClassPath, ESearchCase::CaseSensitive) < 0;
	});

	return Descriptors;
}

// 현재 등록 descriptor 수를 반환합니다.
int32 FCFDATypeRegistry::Num() const
{
	return DescriptorsByClassPath.Num();
}

// exact 동일 descriptor인지 비교합니다.
bool FCFDATypeRegistry::AreDescriptorsEquivalent(
	const FCFDASemanticDescriptor& Left,
	const FCFDASemanticDescriptor& Right)
{
	return Left.ClassPath == Right.ClassPath
		&& Left.Domain == Right.Domain
		&& Left.TypeDisplayName == Right.TypeDisplayName
		&& Left.UserPurposeDescription == Right.UserPurposeDescription
		&& Left.UserUsageDescription == Right.UserUsageDescription
		&& Left.RoleDescription == Right.RoleDescription
		&& Left.IdentityPolicy == Right.IdentityPolicy
		&& Left.IdentityResolverKind == Right.IdentityResolverKind
		&& Left.IdentitySourceName == Right.IdentitySourceName
		&& Left.ValidationPolicy == Right.ValidationPolicy
		&& Left.ValidationSourceName == Right.ValidationSourceName;
}
