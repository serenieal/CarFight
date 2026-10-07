// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentProvider.cpp
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS provider registration seam과 read-only schema fingerprint implementation입니다.
// Changelog:
// - v1.2.0: P0-06 whole-Catalog Snapshot용 deterministic ProviderSchemaFingerprint를 추가.
// - v1.1.0: WorkbookSourceId가 필수인 canonical schema validator에 맞춰 provider schema probe source identity를 고정.
// - v1.0.0: ContentType exact1 provider registry, descriptor validation과 duplicate fail-closed guard를 최초 구현.
// Migration:
// - Product provider adapter는 아직 등록하지 않습니다.
// - Registry는 provider schema seam만 소유하고 Product mutation sequencing은 기존 typed writer가 계속 소유합니다.

#include "DataAuthoring/CFContentProvider.h"

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"

bool FCFContentProviderRegistry::RegisterProvider(
	const TSharedRef<ICFContentProvider>& Provider,
	FString& OutError)
{
	OutError.Reset();

	// Provider가 공개한 stable schema descriptor입니다.
	FCFContentProviderDescriptor Descriptor;
	if (!Provider->DescribeSchema(Descriptor, OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Provider DescribeSchema가 실패했습니다.");
		}
		return false;
	}

	if (!CFIsStableContentId(Descriptor.ProviderId))
	{
		OutError = TEXT("ProviderId가 canonical stable ID가 아닙니다.");
		return false;
	}
	if (!Descriptor.ContentTypeId.IsValid())
	{
		OutError = TEXT("Provider ContentTypeId가 canonical stable ID가 아닙니다.");
		return false;
	}
	if (Descriptor.SchemaRevision <= 0)
	{
		OutError = TEXT("Provider SchemaRevision은 1 이상의 positive integer여야 합니다.");
		return false;
	}
	if (ProvidersByContentType.Contains(Descriptor.ContentTypeId.Value))
	{
		OutError = FString::Printf(
			TEXT("Duplicate ContentType provider registration: %s"),
			*Descriptor.ContentTypeId.Value);
		return false;
	}

	// Descriptor sheets 자체를 공통 Core validator로 검증하는 temporary in-memory model입니다.
	FCFContentWorkbookModel SchemaProbe;
	SchemaProbe.WorkbookSourceId = TEXT("Provider.SchemaProbe");
	SchemaProbe.SchemaRevision = Descriptor.SchemaRevision;
	SchemaProbe.Sheets = Descriptor.Sheets;

	// Common schema validation diagnostics입니다.
	TArray<FCFContentValidationIssue> SchemaIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(SchemaProbe, SchemaIssues))
	{
		OutError = SchemaIssues.Num() > 0
			? FString::Printf(
				TEXT("Provider schema validation 실패: %s (%s)"),
				*SchemaIssues[0].Code,
				*SchemaIssues[0].Path)
			: TEXT("Provider schema validation 실패.");
		return false;
	}

	for (const FCFContentSheetDescriptor& Sheet : Descriptor.Sheets)
	{
		if (!(Sheet.ContentTypeId == Descriptor.ContentTypeId))
		{
			OutError = TEXT("Provider descriptor 안의 모든 sheet는 provider ContentTypeId를 공유해야 합니다.");
			return false;
		}
	}

	ProvidersByContentType.Add(Descriptor.ContentTypeId.Value, Provider);
	return true;
}

ICFContentProvider* FCFContentProviderRegistry::FindProvider(
	const FCFContentTypeId& ContentTypeId) const
{
	// Exact ContentTypeId provider entry입니다.
	const TSharedRef<ICFContentProvider>* Provider = ProvidersByContentType.Find(ContentTypeId.Value);
	return Provider != nullptr ? &Provider->Get() : nullptr;
}

bool FCFContentProviderRegistry::ValidateRegistry(FString& OutError) const
{
	OutError.Reset();

	for (const TPair<FString, TSharedRef<ICFContentProvider>>& Pair : ProvidersByContentType)
	{
		// Registry entry provider descriptor입니다.
		FCFContentProviderDescriptor Descriptor;
		if (!Pair.Value->DescribeSchema(Descriptor, OutError))
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("Registered provider DescribeSchema가 실패했습니다.");
			}
			return false;
		}

		if (!Pair.Key.Equals(Descriptor.ContentTypeId.Value, ESearchCase::CaseSensitive)
			|| !Descriptor.ContentTypeId.IsValid()
			|| !CFIsStableContentId(Descriptor.ProviderId)
			|| Descriptor.SchemaRevision <= 0)
		{
			OutError = FString::Printf(
				TEXT("Provider registry key/descriptor split 상태를 발견했습니다: %s"),
				*Pair.Key);
			return false;
		}

		// Registered descriptor common schema validation용 in-memory model입니다.
		FCFContentWorkbookModel SchemaProbe;
		SchemaProbe.WorkbookSourceId = TEXT("Provider.SchemaProbe");
		SchemaProbe.SchemaRevision = Descriptor.SchemaRevision;
		SchemaProbe.Sheets = Descriptor.Sheets;

		// Common schema diagnostics입니다.
		TArray<FCFContentValidationIssue> SchemaIssues;
		if (!FCFContentSchemaValidator::ValidateWorkbook(SchemaProbe, SchemaIssues))
		{
			OutError = SchemaIssues.Num() > 0
				? FString::Printf(
					TEXT("Registered provider schema validation 실패: %s (%s)"),
					*SchemaIssues[0].Code,
					*SchemaIssues[0].Path)
				: TEXT("Registered provider schema validation 실패.");
			return false;
		}
	}
	return true;
}

bool FCFContentProviderRegistry::BuildSchemaFingerprint(
	FString& OutFingerprint,
	FString& OutError) const
{
	OutFingerprint.Reset();
	OutError.Reset();

	if (!ValidateRegistry(OutError))
	{
		return false;
	}

	// Deterministic Provider schema fingerprint byte stream입니다.
	TArray<uint8> CanonicalBytes;
	CFDACommonPrimitives::AppendStringToken(
		CanonicalBytes,
		TEXT("Protocol"),
		TEXT("CarFight.CCAS.ProviderSchema/v1"));

	// Stable ContentTypeId order의 registered provider keys입니다.
	TArray<FString> ContentTypeIds;
	ProvidersByContentType.GetKeys(ContentTypeIds);
	ContentTypeIds.Sort();

	for (const FString& ContentTypeId : ContentTypeIds)
	{
		// Exact ContentTypeId의 registered provider입니다.
		const TSharedRef<ICFContentProvider>* Provider = ProvidersByContentType.Find(ContentTypeId);
		if (Provider == nullptr)
		{
			OutError = FString::Printf(TEXT("Provider registry entry를 찾지 못했습니다: %s"), *ContentTypeId);
			return false;
		}

		// Fingerprint에 포함할 provider descriptor입니다.
		FCFContentProviderDescriptor Descriptor;
		if (!Provider->Get().DescribeSchema(Descriptor, OutError))
		{
			if (OutError.IsEmpty())
			{
				OutError = FString::Printf(TEXT("Provider DescribeSchema가 실패했습니다: %s"), *ContentTypeId);
			}
			return false;
		}

		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ProviderId"), Descriptor.ProviderId);
		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ContentTypeId"), Descriptor.ContentTypeId.Value);
		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("SchemaRevision"), FString::FromInt(Descriptor.SchemaRevision));

		// Physical registration order와 독립적인 stable SheetId order입니다.
		TArray<const FCFContentSheetDescriptor*> SortedSheets;
		for (const FCFContentSheetDescriptor& Sheet : Descriptor.Sheets)
		{
			SortedSheets.Add(&Sheet);
		}
		SortedSheets.Sort([](const FCFContentSheetDescriptor& Left, const FCFContentSheetDescriptor& Right)
		{
			return Left.SheetId < Right.SheetId;
		});

		for (const FCFContentSheetDescriptor* Sheet : SortedSheets)
		{
			if (Sheet == nullptr)
			{
				continue;
			}

			CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("SheetId"), Sheet->SheetId);
			CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("SheetRevision"), FString::FromInt(Sheet->SchemaRevision));
			CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("SheetType"), Sheet->ContentTypeId.Value);
			CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ParentSheetId"), Sheet->ParentSheetId);
			CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("CollectionId"), Sheet->CollectionId);
			CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("CollectionKind"), FString::FromInt(static_cast<int32>(Sheet->CollectionKind)));

			// Presentation label을 제외한 stable ColumnId order의 semantic fields입니다.
			TArray<const FCFContentFieldDescriptor*> SortedFields;
			for (const FCFContentFieldDescriptor& Field : Sheet->Fields)
			{
				SortedFields.Add(&Field);
			}
			SortedFields.Sort([](const FCFContentFieldDescriptor& Left, const FCFContentFieldDescriptor& Right)
			{
				return Left.ColumnId < Right.ColumnId;
			});

			for (const FCFContentFieldDescriptor* Field : SortedFields)
			{
				if (Field == nullptr)
				{
					continue;
				}

				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ColumnId"), Field->ColumnId);
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ValueType"), FString::FromInt(static_cast<int32>(Field->ValueType)));
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("Ownership"), FString::FromInt(static_cast<int32>(Field->Ownership)));
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ExternalOwnerId"), Field->ExternalOwnerId);
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("CanonicalUnitId"), Field->CanonicalUnitId);
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("FieldDomainId"), Field->FieldDomainId);
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("TextMode"), FString::FromInt(static_cast<int32>(Field->TextMode)));
				CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("AllowNone"), Field->bAllowNone);
				CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("Required"), Field->bRequired);
			}
		}
	}

	return CFDACommonPrimitives::HashCanonicalBytes(
		CanonicalBytes,
		OutFingerprint,
		OutError);
}
