// Copyright (c) CarFight. All Rights Reserved.
// File: CFDACommonPrimitives.h
// Version: v1.1.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-02 provider-neutral Authoring primitive authority입니다.
// Changelog:
// - v1.1.0: common envelope의 TargetObjectPath canonical validation과 BaseSemanticFingerprint physical parse/validation authority를 추가해 Missile/Ammo 중복 구현을 제거.
// - v1.0.0: shared issue/JSON/Literal FText/fingerprint/SoftObjectPath/Asset Registry metadata-only validation seam을 추가.
// Migration:
// - Editor Private internal helper입니다. Public DataAuthoring API와 runtime/Blueprint/DataAsset 계약은 변경하지 않습니다.
// - provider-specific payload shape/range/enum/identity 정책은 이 파일로 이동하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFDAStaging.h"
#include "Dom/JsonObject.h"
#include "UObject/SoftObjectPath.h"

namespace CFDACommonPrimitives
{
	// blocking 또는 informational issue를 target 배열에 추가합니다.
	void AddIssue(
		TArray<FCFDAStagingIssue>& OutIssues,
		ECFDAStagingIssueCode Code,
		const FString& FieldPath,
		const FString& Message,
		bool bBlocking = true);

	// issue 배열에 blocking 진단이 하나라도 존재하는지 확인합니다.
	bool HasBlockingIssue(const TArray<FCFDAStagingIssue>& Issues);

	// semantic string token을 accepted length-prefixed byte protocol로 append합니다.
	void AppendStringToken(TArray<uint8>& OutBytes, const TCHAR* Label, const FString& Value);

	// semantic bool token을 accepted byte protocol로 append합니다.
	void AppendBoolToken(TArray<uint8>& OutBytes, const TCHAR* Label, bool bValue);

	// semantic float token을 IEEE-754 big-endian accepted byte protocol로 append합니다.
	void AppendFloatToken(TArray<uint8>& OutBytes, const TCHAR* Label, float Value);

	// FName semantic equality와 맞추기 위해 comparison text를 lowercase로 canonicalize합니다.
	FString CanonicalNameText(FName Value);

	// SHA-256 canonical lowercase `sha256:<64 hex>` 형식인지 확인합니다.
	bool IsCanonicalSha256Fingerprint(const FString& Fingerprint);

	// canonical byte stream을 SHA-256 protocol fingerprint로 변환합니다.
	bool HashCanonicalBytes(const TArray<uint8>& Bytes, FString& OutFingerprint, FString& OutError);

	// provider-local typed payload에서 다시 계산한 fingerprint와 cached fingerprint의 exact integrity를 검증합니다.
	bool ValidateCachedSemanticFingerprint(
		const FString& CachedFingerprint,
		const FString& RecomputedFingerprint,
		FString& OutError);

	// common envelope TargetObjectPath가 exact `/Game/.../Asset.Asset` canonical object path인지 검사합니다.
	bool ValidateTargetObjectPath(
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues);

	// common envelope BaseSemanticFingerprint의 required null/string physical contract와 canonical SHA-256을 parse합니다.
	void ParseBaseSemanticFingerprint(
		const TSharedPtr<FJsonObject>& RootObject,
		bool& bOutHasBaseSemanticFingerprint,
		FString& OutBaseSemanticFingerprint,
		TArray<FCFDAStagingIssue>& OutIssues);

	// JSON object가 exact required field set만 가지는지 검사합니다.
	bool ValidateExactFields(
		const TSharedPtr<FJsonObject>& Object,
		const TArray<FString>& RequiredFields,
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues);

	// exact field를 찾아 expected JSON type인지 검사합니다.
	TSharedPtr<FJsonValue> RequireField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		EJson ExpectedType,
		TArray<FCFDAStagingIssue>& OutIssues);

	// required JSON string field를 parse합니다.
	bool ParseStringField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		FString& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues);

	// required JSON integer-valued number field를 parse합니다.
	bool ParseRevisionField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		int32& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues);

	// required JSON bool field를 parse합니다.
	bool ParseBoolField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		bool& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues);

	// required JSON number를 requested finite float range로 strict parse합니다.
	bool ParseFloatField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		double MinimumValue,
		double MaximumValue,
		float& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues);

	// required JSON object field를 parse합니다.
	bool ParseObjectField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		TSharedPtr<FJsonObject>& OutObject,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Literal FText object를 strict `{Kind,Text}` shape로 parse합니다.
	bool ParseLiteralText(
		const TSharedPtr<FJsonObject>& TextObject,
		const FString& FieldPath,
		FCFDAStagingLiteralText& OutText,
		TArray<FCFDAStagingIssue>& OutIssues);

	// persisted FText가 Literal interchange로 loss 없이 표현 가능한지 검사하고 source codepoint sequence를 읽습니다.
	bool ReadLiteralTextFromAsset(
		const FText& SourceText,
		const FString& FieldPath,
		FCFDAStagingLiteralText& OutText,
		TArray<FCFDAStagingIssue>& OutIssues);

	// nullable string을 load 없이 canonical top-level SoftObjectPath로 strict parse합니다.
	bool ParseCanonicalSoftObjectPath(
		const FString& PathText,
		const FString& FieldPath,
		bool bAllowNull,
		FSoftObjectPath& OutObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues);

	// SoftObjectPath referenced asset의 존재와 base-class compatibility를 Asset Registry metadata만으로 검사합니다.
	bool ValidateAssetReferenceMetadata(
		const FSoftObjectPath& ObjectPath,
		const FTopLevelAssetPath& ExpectedBaseClassPath,
		const FString& FieldPath,
		TArray<FCFDAStagingIssue>& OutIssues);

#if WITH_DEV_AUTOMATION_TESTS
	// 한 fingerprint probe lifetime 동안 현재 thread의 token observation sink를 설치하고 이전 상태로 복원합니다.
	class FScopedSemanticTokenProbe final
	{
	public:
		// 현재 thread에 기존 probe가 없을 때 requested sink를 설치합니다.
		explicit FScopedSemanticTokenProbe(TArray<FString>& RequestedSink);

		// Scope 종료 시 이전 sink를 복원합니다.
		~FScopedSemanticTokenProbe();

		// 이번 scope가 probe sink ownership을 획득했는지 반환합니다.
		bool IsBound() const;

	private:
		// Scope 진입 전 현재 thread의 probe sink입니다.
		TArray<FString>* PreviousSink = nullptr;

		// 이번 scope가 실제 sink ownership을 획득했는지 나타냅니다.
		bool bBound = false;
	};
#endif
}
