// Copyright (c) CarFight. All Rights Reserved.
// File: CFDATypeRegistry.h
// Version: v1.1.2
// Date: 2026-09-03
// Description: CF-FQ-045 Typed Semantic Registry + current concrete type semantic mapping entry입니다.
// Changelog:
// - v1.1.2: duplicate namespace derivation policy resolver를 추가.
// - v1.1.1: IdentityPolicy Required/Optional/N/A와 IdentityResolverKind 분리 계약을 반영.
// - v1.1.0: DAM-P0-02B current concrete CarFight descriptor 등록 entry와 Identity/Validation policy metadata 계약을 추가.
// - v1.0.0: semantic descriptor 등록/lookup, deterministic enumeration, Coverage refresh bridge, Unregistered fallback을 추가.
// Migration:
// - RegisterCurrentCarFightDescriptors()는 descriptor metadata만 등록하며 Asset load, Stable ID resolve, Validation 실행을 하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementTypes.h"

// DataAsset class path와 사용자 semantic descriptor를 연결하는 Editor-only Registry입니다.
class CARFIGHT_REEDITOR_API FCFDATypeRegistry
{
public:
	// current concrete CarFight DataAsset 전체의 audited semantic/policy descriptor를 이 Registry에 등록합니다.
	bool RegisterCurrentCarFightDescriptors(FString* OutError = nullptr);

	// descriptor를 exact class path 기준으로 등록합니다. 동일 descriptor 재등록은 idempotent하게 허용합니다.
	bool RegisterDescriptor(
		const FCFDASemanticDescriptor& Descriptor,
		FString* OutError = nullptr);

	// exact class path에 등록된 descriptor를 찾습니다.
	const FCFDASemanticDescriptor* FindDescriptor(const FString& ClassPath) const;

	// 등록 여부와 안전한 Unregistered fallback을 함께 반환합니다.
	FCFDAResolvedSemantic ResolveSemantic(
		const FString& ClassPath,
		const FString& TechnicalClassName) const;

	// descriptor의 identity 계약에서 duplicate 비교 namespace 정책을 deterministic하게 유도합니다.
	static ECFDADuplicateNamespacePolicy ResolveDuplicateNamespacePolicy(
		const FCFDASemanticDescriptor& Descriptor);

	// 현재 Registry coverage를 P0-01 metadata Refresh 옵션으로 변환합니다.
	FCFDARefreshOptions BuildRefreshOptions() const;

	// 등록 descriptor를 canonical class path 순으로 반환합니다.
	TArray<FCFDASemanticDescriptor> GetDescriptors() const;

	// 현재 등록 descriptor 수를 반환합니다.
	int32 Num() const;

private:
	// exact 동일 descriptor인지 비교합니다.
	static bool AreDescriptorsEquivalent(
		const FCFDASemanticDescriptor& Left,
		const FCFDASemanticDescriptor& Right);

	// canonical class path별 semantic descriptor 저장소입니다.
	TMap<FString, FCFDASemanticDescriptor> DescriptorsByClassPath;
};
