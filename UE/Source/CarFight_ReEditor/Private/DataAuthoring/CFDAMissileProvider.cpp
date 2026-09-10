// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAMissileProvider.cpp
// Version: v1.3.0
// Date: 2026-09-10
// Description: CF-FQ-051 MissileGuidePreset authoring provider와 accepted per-TypeKey DACE boundary를 구성합니다.
// Changelog:
// - v1.3.0: DAO-P0-04 correction에서 Missile DACE를 ContractReady로 명시하고 기존 Product canonical exact3 경로를 provider-owned target set으로 투영했습니다.
// - v1.2.0: 기존 Missile provider를 ReviewedMutationReady로 명시해 P0-01 Apply semantics를 보존.
// - v1.1.0: exact TypeKey descriptor에 Stable Identity policy/resolver, DACE owner/history namespace와 shared parse/current/apply operation table을 결속.
// - v1.0.0: accepted Missile schema/class/revision/root/identity와 existing production callback entry를 하나의 typed provider authority로 결합.
// Migration:
// - 기존 Missile schema/class/revision/root/semantic callback 값은 변경하지 않습니다. read-only second provider가 추가되어도 Missile은 Reviewed Apply 가능 상태를 유지합니다.
// - v1.3.0의 DACE metadata projection은 기존 CFDAContractBase.cpp accepted history와 canonical exact3 파일을 수정하지 않습니다.

#include "CFDAMissileProvider.h"

// Current MissileGuidePreset typed provider authority를 반환합니다.
const FCFDAMissileTypeProvider& CFDAMissileProvider::GetProvider()
{
	// CF-FQ-049/DACE accepted MissileGuidePreset metadata + shared/typed behavior authority입니다.
	static const FCFDAMissileTypeProvider MissileProvider = []()
	{
		// 완성할 first production provider entry입니다.
		FCFDAMissileTypeProvider Provider;
		Provider.Descriptor.TypeKey.SchemaId = TEXT("CarFight.DataAsset.MissileGuidePreset");
		Provider.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFMissileGuidePresetData");
		Provider.Descriptor.SchemaRevision = 1;
		Provider.Descriptor.AdapterContractRevision = 2;
		Provider.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging/MissileGuidePreset");
		Provider.Descriptor.StableIdentityPolicy = ECFDAStableIdentityPolicy::Required;
		Provider.Descriptor.StableIdentityResolver = ECFDAStableIdentityResolver::ExplicitFName;
		Provider.Descriptor.StableIdentitySourceName = FName(TEXT("PresetId"));
		Provider.Descriptor.DaceContractOwnerName = FName(TEXT("FCFDAContractGuard"));
		Provider.Descriptor.DaceAcceptedHistoryNamespace = TEXT("DACE-MissileGuidePreset");
		Provider.Descriptor.DaceReadiness = ECFDADaceReadiness::ContractReady;
		Provider.Descriptor.bDaceCanonicalStagingTargetSetDeclared = true;
		Provider.Descriptor.DaceCanonicalStagingRelativePaths =
		{
			TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json"),
			TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Normal.json"),
			TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_High.json")
		};
		Provider.Readiness = ECFDAProviderReadiness::ReviewedMutationReady;

		Provider.Operations.ParseCommonCandidate = &CFDAMissileProviderImpl::ParseCommonCandidate;
		Provider.Operations.ResolveCommonCurrentState = &CFDAMissileProviderImpl::ResolveCommonCurrentState;
		Provider.Operations.ApplyReviewedMutation = &CFDAMissileProviderImpl::ApplyReviewedMutation;

		Provider.Callbacks.ParseJson = &CFDAMissileProviderImpl::ParseJson;
		Provider.Callbacks.BuildSemanticFingerprint = &CFDAMissileProviderImpl::BuildSemanticFingerprint;
		Provider.Callbacks.ExtractPayload = &CFDAMissileProviderImpl::ExtractPayload;
		Provider.Callbacks.ResolveCurrentState = &CFDAMissileProviderImpl::ResolveCurrentState;
		Provider.Callbacks.SerializeProductStagingJson = &CFDAMissileProviderImpl::SerializeProductStagingJson;
		Provider.Callbacks.MaterializePayload = &CFDAMissileProviderImpl::MaterializePayload;
		return Provider;
	}();
	return MissileProvider;
}
