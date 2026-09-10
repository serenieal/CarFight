// Copyright (c) CarFight. All Rights Reserved.
// File: CFDATypeDispatch.cpp
// Version: v1.6.0
// Date: 2026-09-10
// Description: CF-FQ-051 Missile + Ammo exact TypeKey provider registry, mutation readiness와 DACE boundary 구현입니다.
// Changelog:
// - v1.6.0: DAO-P0-05에서 production registry를 노출하지 않고 exact Staging path의 provider-owned root owner를 JSON read 전에 하나만 확정하는 resolver와 overlapping-root test seam을 추가했습니다.
// - v1.5.0: DAO-P0-04 correction에서 DACE readiness와 explicit per-TypeKey canonical Staging target-set validation을 추가해 empty exact0과 미선언 상태를 분리했습니다.
// - v1.4.0: DAO-P0-03에서 AmmoData를 ReviewedMutationReady로 승격해 Missile + Ammo production provider exact2가 동일 readiness gate를 통과하도록 current projection 갱신.
// - v1.3.0: AmmoData exact TypeKey provider를 ReadOnlyPreviewReady로 production registry에 추가하고 Missile ReviewedMutationReady와 독립 readiness를 유지.
// - v1.2.0: provider readiness를 ReadOnlyPreviewReady / ReviewedMutationReady로 분리하고 read-only provider registry 허용 + Apply fail-closed validation을 추가.
// - v1.1.0: descriptor-only first-match registry를 complete provider entry registry로 교체하고 duplicate TypeKey/incomplete callback을 fail-closed. Stable Identity policy/resolver와 DACE owner/history authority를 registration validation에 결속하고 sibling Automation root allowlist를 제거.
// - v1.0.2: 기존 focused Automation sibling fixture root allowlist를 임시 추가했던 historical implementation.
// - v1.0.0: MissileGuidePreset exact provider authority와 provider-owned StagingRoot containment을 최초 구현.
// Migration:
// - CF-FQ-049 Automation fixture는 Missile provider root 하위 `MissileGuidePreset/__Automation...__`를 유지합니다.
// - ReadOnlyPreviewReady provider는 Parse/Current만 요구하고 Apply callback은 금지합니다. ReviewedMutationReady provider만 Apply callback을 소유할 수 있습니다.
// - v1.5.0부터 DACE canonical target set은 provider별 explicit declaration입니다. 선언된 empty set은 valid exact0이며 DACE readiness와 authoring readiness는 독립입니다.
// - v1.6.0부터 mixed operational selection은 JSON payload의 TypeKey를 읽기 전에 full registered-provider root ownership을 먼저 exact1로 확정하고, 그 뒤 caller allowed TypeKey scope를 검증합니다.

#include "CFDATypeDispatch.h"
#include "CFDAAmmoProvider.h"
#include "CFDAMissileProvider.h"

namespace CFDATypeDispatchPrivate
{
	// Production exact provider registry입니다. JSON/DTO가 implementation을 직접 선택하지 못하도록 code-owned list로 고정합니다.
	const TArray<const FCFDATypeProviderEntry*>& GetRegisteredProviders()
	{
		// DAO-P0-03 implementation 시점 production provider는 Missile + Ammo ReviewedMutationReady exact2입니다.
		static const TArray<const FCFDATypeProviderEntry*> RegisteredProviders =
		{
			&CFDAMissileProvider::GetProvider(),
			&CFDAAmmoProvider::GetProvider()
		};
		return RegisteredProviders;
	}

	// 한 provider entry가 선언된 readiness 단계에 필요한 complete authority를 제공하는지 검사합니다.
	bool ValidateProviderEntry(const FCFDATypeProviderEntry* ProviderEntry, FString& OutError)
	{
		if (ProviderEntry == nullptr)
		{
			OutError = TEXT("Provider registry에 null entry가 존재합니다.");
			return false;
		}

		// 검증할 provider structural descriptor입니다.
		const FCFDATypeProvider& Provider = ProviderEntry->Descriptor;
		if (Provider.TypeKey.SchemaId.IsEmpty()
			|| Provider.TypeKey.DataAssetTypeClassPath.IsEmpty()
			|| Provider.SchemaRevision <= 0
			|| Provider.AdapterContractRevision <= 0
			|| Provider.CanonicalStagingRoot.IsEmpty()
			|| Provider.StableIdentitySourceName.IsNone()
			|| Provider.DaceContractOwnerName.IsNone()
			|| Provider.DaceAcceptedHistoryNamespace.IsEmpty())
		{
			OutError = TEXT("Provider structural/identity/DACE authority가 complete하지 않습니다.");
			return false;
		}
		if (Provider.StableIdentityPolicy != ECFDAStableIdentityPolicy::Required
			|| Provider.StableIdentityResolver != ECFDAStableIdentityResolver::ExplicitFName)
		{
			OutError = TEXT("P0 provider StableLogicalId policy/resolver가 Required + ExplicitFName 계약과 다릅니다.");
			return false;
		}
		if (!Provider.bDaceCanonicalStagingTargetSetDeclared)
		{
			OutError = TEXT("Provider가 DACE canonical Staging target set을 explicit하게 선언하지 않았습니다.");
			return false;
		}

		// Provider-local canonical target path의 normalized duplicate를 차단하는 집합입니다.
		TSet<FString> NormalizedDaceTargetPaths;
		for (const FString& DaceTargetPath : Provider.DaceCanonicalStagingRelativePaths)
		{
			// Provider root containment과 `.json` canonical form을 통과한 target path입니다.
			FString NormalizedDaceTargetPath;
			if (!CFDATypeDispatch::NormalizeProviderStagingPath(Provider, DaceTargetPath, NormalizedDaceTargetPath))
			{
				OutError = TEXT("Provider DACE canonical Staging target이 selected provider root 아래 canonical `.json` 경로가 아닙니다.");
				return false;
			}
			if (NormalizedDaceTargetPaths.Contains(NormalizedDaceTargetPath))
			{
				OutError = TEXT("Provider DACE canonical Staging target set에 duplicate path가 있습니다.");
				return false;
			}
			NormalizedDaceTargetPaths.Add(NormalizedDaceTargetPath);
		}
		if (Provider.DaceReadiness != ECFDADaceReadiness::ContractNotReady
			&& Provider.DaceReadiness != ECFDADaceReadiness::ContractReady)
		{
			OutError = TEXT("Provider DACE readiness 값이 지원 범위 밖입니다.");
			return false;
		}
		if (ProviderEntry->Operations.ParseCommonCandidate == nullptr
			|| ProviderEntry->Operations.ResolveCommonCurrentState == nullptr)
		{
			OutError = TEXT("Provider read-only Preview operation table이 complete하지 않습니다.");
			return false;
		}

		if (ProviderEntry->Readiness == ECFDAProviderReadiness::ReadOnlyPreviewReady)
		{
			if (ProviderEntry->Operations.ApplyReviewedMutation != nullptr)
			{
				OutError = TEXT("ReadOnlyPreviewReady provider가 mutation callback을 보유하고 있습니다.");
				return false;
			}
		}
		else if (ProviderEntry->Readiness == ECFDAProviderReadiness::ReviewedMutationReady)
		{
			if (ProviderEntry->Operations.ApplyReviewedMutation == nullptr)
			{
				OutError = TEXT("ReviewedMutationReady provider에 mutation callback이 없습니다.");
				return false;
			}
		}
		else
		{
			OutError = TEXT("Provider readiness 값이 지원 범위 밖입니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Explicit provider set에서 incomplete entry와 duplicate exact TypeKey를 fail-closed합니다.
	bool ValidateProviderSet(
		const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
		FString& OutError)
	{
		if (ProviderEntries.IsEmpty())
		{
			OutError = TEXT("Provider registry가 비어 있습니다.");
			return false;
		}

		// 이미 관측한 exact TypeKey canonical key 집합입니다.
		TSet<FString> SeenTypeKeys;
		for (const FCFDATypeProviderEntry* ProviderEntry : ProviderEntries)
		{
			if (!ValidateProviderEntry(ProviderEntry, OutError))
			{
				return false;
			}

			// Case-sensitive SchemaId/class pair를 lossless separator로 결합한 duplicate key입니다.
			const FString ExactTypeKey = ProviderEntry->Descriptor.TypeKey.SchemaId
				+ TEXT("\x1f")
				+ ProviderEntry->Descriptor.TypeKey.DataAssetTypeClassPath;
			if (SeenTypeKeys.Contains(ExactTypeKey))
			{
				OutError = TEXT("Provider registry에 duplicate exact TypeKey가 존재합니다.");
				return false;
			}
			SeenTypeKeys.Add(ExactTypeKey);
		}

		OutError.Reset();
		return true;
	}

	// Caller가 허용한 TypeKey scope에 selected provider의 exact key가 포함되는지 검사합니다.
	bool IsProviderTypeKeyAllowed(
		const FCFDATypeProvider& Provider,
		const TArray<FCFDATypeKey>& AllowedTypeKeys)
	{
		for (const FCFDATypeKey& AllowedTypeKey : AllowedTypeKeys)
		{
			if (Provider.TypeKey.SchemaId.Equals(AllowedTypeKey.SchemaId, ESearchCase::CaseSensitive)
				&& Provider.TypeKey.DataAssetTypeClassPath.Equals(AllowedTypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
			{
				return true;
			}
		}
		return false;
	}

	// Explicit provider set 전체에서 path containment owner를 exact1로 찾은 뒤 allowed TypeKey scope를 검증합니다.
	const FCFDATypeProviderEntry* FindStagingPathOwnerInSet(
		const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
		const FString& InputPath,
		const TArray<FCFDATypeKey>& AllowedTypeKeys,
		FString& OutNormalizedPath,
		FString* OutError)
	{
		OutNormalizedPath.Reset();
		// Provider set 자체가 trusted complete registry인지 확인한 오류입니다.
		FString RegistryError;
		if (!ValidateProviderSet(ProviderEntries, RegistryError))
		{
			if (OutError != nullptr)
			{
				*OutError = RegistryError;
			}
			return nullptr;
		}
		if (AllowedTypeKeys.IsEmpty())
		{
			if (OutError != nullptr)
			{
				*OutError = TEXT("Staging path owner resolver의 explicit allowed TypeKey scope가 비어 있습니다.");
			}
			return nullptr;
		}

		// Full provider set에서 root containment을 만족한 owner 후보입니다.
		const FCFDATypeProviderEntry* MatchedProvider = nullptr;
		// Exact owner 후보가 반환한 canonical normalized path입니다.
		FString MatchedNormalizedPath;
		// Full registry에서 path를 소유한다고 판정한 provider 수입니다.
		int32 OwnerCount = 0;
		for (const FCFDATypeProviderEntry* ProviderEntry : ProviderEntries)
		{
			// 이 provider root 기준 canonical containment 결과입니다.
			FString ProviderNormalizedPath;
			if (!CFDATypeDispatch::NormalizeProviderStagingPath(
				ProviderEntry->Descriptor,
				InputPath,
				ProviderNormalizedPath))
			{
				continue;
			}
			++OwnerCount;
			MatchedProvider = ProviderEntry;
			MatchedNormalizedPath = MoveTemp(ProviderNormalizedPath);
		}

		if (OwnerCount != 1 || MatchedProvider == nullptr)
		{
			if (OutError != nullptr)
			{
				*OutError = OwnerCount == 0
					? TEXT("Exact Staging path를 소유하는 trusted provider root가 없습니다.")
					: TEXT("Exact Staging path가 둘 이상의 trusted provider root에 포함되어 owner가 모호합니다.");
			}
			return nullptr;
		}
		if (!IsProviderTypeKeyAllowed(MatchedProvider->Descriptor, AllowedTypeKeys))
		{
			if (OutError != nullptr)
			{
				*OutError = TEXT("Exact Staging path owner가 caller의 explicit allowed TypeKey scope에 포함되지 않습니다.");
			}
			return nullptr;
		}

		OutNormalizedPath = MoveTemp(MatchedNormalizedPath);
		if (OutError != nullptr)
		{
			OutError->Reset();
		}
		return MatchedProvider;
	}

	// Explicit provider set에서 exact TypeKey를 하나만 선택합니다.
	const FCFDATypeProviderEntry* FindExactProviderInSet(
		const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
		const FString& SchemaId,
		const FString& DataAssetTypeClassPath,
		FString* OutError)
	{
		// Provider set structural validation 실패 원인입니다.
		FString RegistryError;
		if (!ValidateProviderSet(ProviderEntries, RegistryError))
		{
			if (OutError != nullptr)
			{
				*OutError = RegistryError;
			}
			return nullptr;
		}

		// Exact TypeKey match 결과입니다.
		const FCFDATypeProviderEntry* MatchedProvider = nullptr;
		for (const FCFDATypeProviderEntry* ProviderEntry : ProviderEntries)
		{
			if (ProviderEntry->Descriptor.TypeKey.SchemaId.Equals(SchemaId, ESearchCase::CaseSensitive)
				&& ProviderEntry->Descriptor.TypeKey.DataAssetTypeClassPath.Equals(DataAssetTypeClassPath, ESearchCase::CaseSensitive))
			{
				MatchedProvider = ProviderEntry;
				break;
			}
		}

		if (OutError != nullptr)
		{
			if (MatchedProvider == nullptr)
			{
				*OutError = TEXT("Exact TypeKey에 등록된 trusted provider가 없습니다.");
			}
			else
			{
				OutError->Reset();
			}
		}
		return MatchedProvider;
	}
}

// Current MissileGuidePreset complete trusted provider entry를 반환합니다.
const FCFDATypeProviderEntry& CFDATypeDispatch::GetMissilePresetProviderEntry()
{
	return CFDAMissileProvider::GetProvider();
}

// Current MissileGuidePreset trusted provider descriptor compatibility view를 반환합니다.
const FCFDATypeProvider& CFDATypeDispatch::GetMissilePresetProvider()
{
	return CFDAMissileProvider::GetProvider().Descriptor;
}

// Production provider registry가 duplicate TypeKey와 readiness-incomplete authority를 포함하지 않는지 fail-closed 검증합니다.
bool CFDATypeDispatch::ValidateProviderRegistry(FString& OutError)
{
	return CFDATypeDispatchPrivate::ValidateProviderSet(CFDATypeDispatchPrivate::GetRegisteredProviders(), OutError);
}

// Exact SchemaId + DataAssetTypeClassPath TypeKey에 등록된 readiness-valid provider entry만 반환합니다.
const FCFDATypeProviderEntry* CFDATypeDispatch::FindExactProviderEntry(
	const FString& SchemaId,
	const FString& DataAssetTypeClassPath,
	FString* OutError)
{
	return CFDATypeDispatchPrivate::FindExactProviderInSet(
		CFDATypeDispatchPrivate::GetRegisteredProviders(),
		SchemaId,
		DataAssetTypeClassPath,
		OutError);
}

// Reviewed Apply 진입 전 selected provider가 mutation-ready인지 fail-closed 확인합니다.
bool CFDATypeDispatch::ValidateProviderMutationReady(
	const FCFDATypeProviderEntry& ProviderEntry,
	FString& OutError)
{
	// provider 자체의 declared readiness/operation consistency 오류입니다.
	FString ProviderValidationError;
	if (!CFDATypeDispatchPrivate::ValidateProviderEntry(&ProviderEntry, ProviderValidationError))
	{
		OutError = ProviderValidationError;
		return false;
	}
	if (ProviderEntry.Readiness != ECFDAProviderReadiness::ReviewedMutationReady
		|| ProviderEntry.Operations.ApplyReviewedMutation == nullptr)
	{
		OutError = TEXT("Selected provider는 read-only Preview까지만 준비됐으며 Reviewed mutation을 허용하지 않습니다.");
		return false;
	}
	OutError.Reset();
	return true;
}

// Common envelope의 exact TypeKey/revision/StagingRoot가 trusted provider 계약과 일치하는지 검사합니다.
bool CFDATypeDispatch::ValidateProviderContract(
	const FCFDACommonEnvelope& Envelope,
	const FCFDATypeProvider& Provider,
	FString& OutError)
{
	if (!Envelope.SchemaId.Equals(Provider.TypeKey.SchemaId, ESearchCase::CaseSensitive)
		|| !Envelope.DataAssetTypeClassPath.Equals(Provider.TypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Common envelope TypeKey가 selected trusted provider와 exact 일치하지 않습니다.");
		return false;
	}
	if (Envelope.SchemaRevision != Provider.SchemaRevision)
	{
		OutError = TEXT("Common envelope SchemaRevision이 selected trusted provider revision과 다릅니다.");
		return false;
	}
	if (Envelope.AdapterContractRevision != Provider.AdapterContractRevision)
	{
		OutError = TEXT("Common envelope AdapterContractRevision이 selected trusted provider revision과 다릅니다.");
		return false;
	}
	if (Provider.StableIdentityPolicy == ECFDAStableIdentityPolicy::Required && Envelope.StableLogicalId.IsNone())
	{
		OutError = TEXT("Common envelope required StableLogicalId가 비어 있습니다.");
		return false;
	}
	if (!Envelope.StagingRelativePath.IsEmpty())
	{
		// Provider root containment 검증 뒤 canonicalized Staging path입니다.
		FString NormalizedStagingPath;
		if (!NormalizeProviderStagingPath(Provider, Envelope.StagingRelativePath, NormalizedStagingPath)
			|| !NormalizedStagingPath.Equals(Envelope.StagingRelativePath, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Common envelope StagingRelativePath가 selected trusted provider의 canonical StagingRoot 밖에 있습니다.");
			return false;
		}
	}
	OutError.Reset();
	return true;
}

// Provider-owned StagingRoot 아래의 canonical lowercase .json source path로 정규화합니다.
bool CFDATypeDispatch::NormalizeProviderStagingPath(
	const FCFDATypeProvider& Provider,
	const FString& InputPath,
	FString& OutPath)
{
	OutPath = InputPath;
	OutPath.TrimStartAndEndInline();
	OutPath.ReplaceInline(TEXT("\\"), TEXT("/"), ESearchCase::CaseSensitive);
	while (OutPath.StartsWith(TEXT("./"), ESearchCase::CaseSensitive))
	{
		OutPath = OutPath.Mid(2);
	}
	if (Provider.CanonicalStagingRoot.IsEmpty()
		|| OutPath.IsEmpty()
		|| OutPath.StartsWith(TEXT("/"), ESearchCase::CaseSensitive)
		|| OutPath.EndsWith(TEXT("/"), ESearchCase::CaseSensitive)
		|| OutPath.Contains(TEXT("//"), ESearchCase::CaseSensitive)
		|| OutPath.Contains(TEXT(":"), ESearchCase::CaseSensitive))
	{
		return false;
	}

	// Normalized path segment 목록입니다.
	TArray<FString> Segments;
	OutPath.ParseIntoArray(Segments, TEXT("/"), true);
	for (const FString& Segment : Segments)
	{
		if (Segment.IsEmpty() || Segment == TEXT(".") || Segment == TEXT(".."))
		{
			return false;
		}
	}

	// Provider root 바로 아래 또는 하위의 concrete JSON filename만 허용하는 exact prefix입니다.
	const FString ProviderRootPrefix = Provider.CanonicalStagingRoot + TEXT("/");
	// Exact provider-owned root 아래에 concrete filename이 존재하는지 나타냅니다.
	const bool bUnderTrustedRoot = OutPath.StartsWith(ProviderRootPrefix, ESearchCase::CaseSensitive)
		&& OutPath.Len() > ProviderRootPrefix.Len() + 5;
	if (!bUnderTrustedRoot
		|| !OutPath.EndsWith(TEXT(".json"), ESearchCase::CaseSensitive))
	{
		return false;
	}
	return true;
}

// Explicit allowed TypeKey scope 안에서 exact Staging path owner를 JSON read 전에 production registry 기준으로 하나만 확정합니다.
const FCFDATypeProviderEntry* CFDATypeDispatch::FindProviderForStagingPath(
	const FString& InputPath,
	const TArray<FCFDATypeKey>& AllowedTypeKeys,
	FString& OutNormalizedPath,
	FString* OutError)
{
	return CFDATypeDispatchPrivate::FindStagingPathOwnerInSet(
		CFDATypeDispatchPrivate::GetRegisteredProviders(),
		InputPath,
		AllowedTypeKeys,
		OutNormalizedPath,
		OutError);
}

// StableLogicalId duplicate scope를 exact DataAsset class + canonical FName semantic으로 만듭니다.
FString CFDATypeDispatch::BuildClassScopedStableIdentityKey(
	const FString& DataAssetTypeClassPath,
	const FName StableLogicalId)
{
	if (DataAssetTypeClassPath.IsEmpty() || StableLogicalId.IsNone())
	{
		return FString();
	}
	return DataAssetTypeClassPath + TEXT("\n") + StableLogicalId.ToString().ToLower();
}

// Approval/TOCTOU가 두 common envelope의 exact immutable binding을 비교합니다.
bool CFDATypeDispatch::AreCommonEnvelopesEquivalent(
	const FCFDACommonEnvelope& Left,
	const FCFDACommonEnvelope& Right)
{
	return Left.SchemaId.Equals(Right.SchemaId, ESearchCase::CaseSensitive)
		&& Left.SchemaRevision == Right.SchemaRevision
		&& Left.AdapterContractRevision == Right.AdapterContractRevision
		&& Left.DataAssetTypeClassPath.Equals(Right.DataAssetTypeClassPath, ESearchCase::CaseSensitive)
		&& Left.StableLogicalId == Right.StableLogicalId
		&& Left.TargetObjectPath.Equals(Right.TargetObjectPath, ESearchCase::CaseSensitive)
		&& Left.StagingRelativePath.Equals(Right.StagingRelativePath, ESearchCase::CaseSensitive)
		&& Left.bHasBaseSemanticFingerprint == Right.bHasBaseSemanticFingerprint
		&& Left.BaseSemanticFingerprint.Equals(Right.BaseSemanticFingerprint, ESearchCase::CaseSensitive)
		&& Left.CurrentSemanticFingerprint.Equals(Right.CurrentSemanticFingerprint, ESearchCase::CaseSensitive)
		&& Left.StagingSemanticFingerprint.Equals(Right.StagingSemanticFingerprint, ESearchCase::CaseSensitive)
		&& Left.PlannedOperation == Right.PlannedOperation;
}

#if WITH_DEV_AUTOMATION_TESTS
// Test-owned explicit provider set에서 duplicate TypeKey/readiness-incomplete provider를 production registry mutation 없이 검증합니다.
bool CFDATypeDispatch::ValidateProviderSetForTests(
	const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
	FString& OutError)
{
	return CFDATypeDispatchPrivate::ValidateProviderSet(ProviderEntries, OutError);
}

// Test-owned explicit provider set에서 exact TypeKey lookup seam을 production registry mutation 없이 검증합니다.
const FCFDATypeProviderEntry* CFDATypeDispatch::FindExactProviderEntryInSetForTests(
	const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
	const FString& SchemaId,
	const FString& DataAssetTypeClassPath,
	FString* OutError)
{
	return CFDATypeDispatchPrivate::FindExactProviderInSet(
		ProviderEntries,
		SchemaId,
		DataAssetTypeClassPath,
		OutError);
}

// Test-owned explicit provider set에서 overlapping root를 production registry mutation 없이 검증하는 exact path-owner seam입니다.
const FCFDATypeProviderEntry* CFDATypeDispatch::FindProviderForStagingPathInSetForTests(
	const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
	const FString& InputPath,
	const TArray<FCFDATypeKey>& AllowedTypeKeys,
	FString& OutNormalizedPath,
	FString* OutError)
{
	return CFDATypeDispatchPrivate::FindStagingPathOwnerInSet(
		ProviderEntries,
		InputPath,
		AllowedTypeKeys,
		OutNormalizedPath,
		OutError);
}
#endif
