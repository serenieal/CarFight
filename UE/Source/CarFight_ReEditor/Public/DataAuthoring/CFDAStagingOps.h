// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingOps.h
// Version: v1.4.0
// Date: 2026-09-11
// Description: CF-FQ-049/051/052/053 canonical Product Staging bootstrap/rebase, selected discovery Preview와 reviewed operational session public contract입니다.
// Changelog:
// - v1.4.0: VDR-P0-02에서 VehicleDefenseData DACE acceptance 뒤 mixed explicit session의 production operational scope를 MissileGuidePreset+AmmoData+DamageData+VehicleDefenseData exact4로 전진시켰습니다. Public session API는 변경하지 않습니다.
// - v1.3.0: DDO-P0-04에서 mixed explicit session의 production operational scope를 MissileGuidePreset+AmmoData+DamageData exact3로 전진시키고 actual backing authority를 관측하는 test-only read projection을 추가했습니다.
// - v1.2.0: DAO-P0-05에서 session non-empty exact path selection만 Missile+Ammo provider-neutral mixed discovery로 확장하고 empty Missile whole-root/Public Missile discovery/console shorthand/SyncProduct 호환성을 보존했습니다.
// - v1.1.1: UE 5.8 console registration API correction과 final operational contract version 정렬.
// - v1.1.0: exact Staging selection, reusable Preview→Review session과 test-only Sync rollback fault injection을 추가.
// - v1.0.0: Product Low/Normal/High canonical Staging bootstrap/rebase, deterministic discovery Preview와 mutation BatchPlanHash 결과 DTO를 추가.
// Migration:
// - Product Sync는 Product .uasset을 수정·저장하지 않으며 기존 Staging Update/Conflict/Invalid을 자동 덮어쓰지 않습니다.
// - Preview selection은 canonical repository-relative JSON path exact-list입니다. empty list는 canonical folder 전체를 의미합니다.
// - 실제 UE Asset materialize/save는 reviewed session의 explicit ApplyReviewed 또는 기존 FCFDAStagingApplyService만 소유합니다.
// - v1.4.0부터 session의 non-empty full relative JSON path는 trusted MissileGuidePreset+AmmoData+DamageData+VehicleDefenseData exact4를 mixed 선택할 수 있습니다. operational scope는 provider registry에서 자동 생성하지 않으며 empty selection과 StableLogicalId console shorthand는 기존 Missile 의미를 유지합니다.
// - mixed session의 Rows는 공통 envelope를 기존 Preview DTO에 투영한 구조 요약이며 typed Missile Payload는 authoritative하지 않습니다. 기존 Missile-only discovery/session은 종전 typed row를 그대로 반환합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFDAStaging.h"
#include "DataAuthoring/CFDAStagingApply.h"

// canonical Staging discovery 한 번의 exact Preview projection입니다.
struct FCFDAStagingOpsPreview
{
	// deterministic selected Staging source order로 정렬된 Preview rows입니다.
	TArray<FCFDAStagingPreviewRow> Rows;

	// Create row 수입니다.
	int32 CreateCount = 0;

	// Update row 수입니다.
	int32 UpdateCount = 0;

	// NoChange row 수입니다.
	int32 NoChangeCount = 0;

	// Conflict row 수입니다.
	int32 ConflictCount = 0;

	// Invalid row 수입니다.
	int32 InvalidCount = 0;

	// Conflict/Invalid blocker가 하나라도 있는지 나타냅니다.
	bool bBlocked = false;

	// Create/Update mutation candidate가 하나라도 있는지 나타냅니다.
	bool bHasMutationCandidates = false;

	// mutation candidate가 있고 blocker가 없을 때 계산한 exact BatchPlanHash입니다.
	FString BatchPlanHash;
};

// DAS-P0-05 Editor-side authoring operation을 staging-file 범위에서 재사용 가능하게 연결합니다.
class CARFIGHT_REEDITOR_API FCFDAStagingOps
{
public:
	// P0 MissileGuidePreset canonical repository-relative Staging directory를 반환합니다.
	static const TCHAR* GetMissilePresetStagingRoot();

	// persisted Product Low/Normal/High를 현재 semantic baseline으로 canonical Staging JSON에 bootstrap/rebase하며 Product .uasset은 저장하지 않습니다.
	static bool SyncProductStaging(
		TArray<FString>& OutChangedStagingPaths,
		FString& OutError);

	// canonical MissileGuidePreset Staging JSON 전체를 deterministic discover/parse/resolve/Preview하고 approval용 BatchPlanHash까지 계산합니다.
	static bool DiscoverMissilePresetPreview(
		FCFDAStagingOpsPreview& OutPreview,
		FString& OutError);

	// canonical MissileGuidePreset Staging 중 exact selected relative paths만 deterministic discover/parse/resolve/Preview하고 approval용 BatchPlanHash까지 계산합니다.
	static bool DiscoverMissilePresetPreview(
		const TArray<FString>& SelectedStagingRelativePaths,
		FCFDAStagingOpsPreview& OutPreview,
		FString& OutError);
};

// Preview→fresh Review→explicit Apply one-shot state를 재사용 가능한 운영 단위로 소유합니다.
class CARFIGHT_REEDITOR_API FCFDAStagingOpsSession
{
public:
	// selected exact Staging set을 fresh Preview하고 이후 Review가 같은 selection을 재검증하도록 동결합니다.
	bool Preview(
		const TArray<FString>& SelectedStagingRelativePaths,
		FCFDAStagingOpsPreview& OutPreview,
		FString& OutError);

	// 마지막 mutation Preview를 동일 selection의 fresh discovery/hash와 재대조해 Reviewed approval로 동결합니다.
	bool Review(FString& OutError);

	// current session에 실제 Reviewed one-shot approval이 존재하는지 반환합니다.
	bool HasReviewedApproval() const;

	// 현재 Reviewed approval evidence를 read-only로 반환합니다.
	const FCFDAStagingReviewedApproval& GetReviewedApproval() const;

	// 현재 Reviewed approval을 기존 exact Apply service에 one-shot 전달합니다.
	bool ApplyReviewed(
		FCFDAStagingApplyReport& OutReport,
		FString& OutError);

	// Preview/selection/approval session state를 전부 초기화합니다.
	void Reset();

private:
	// 마지막 Preview가 대상으로 삼은 exact canonical relative path selection입니다. empty면 전체 discovery입니다.
	TArray<FString> SelectedStagingRelativePaths;

	// 마지막 successful Preview projection입니다.
	FCFDAStagingOpsPreview LastPreview;

	// 마지막 non-empty Preview가 provider-neutral mixed Explicit Paths discovery를 사용했는지 나타냅니다.
	bool bLastPreviewUsedMixedExplicitDiscovery = false;

	// 마지막 Preview가 Review 가능한 current session state인지 나타냅니다.
	bool bHasLastPreview = false;

	// fresh Review가 동결한 one-shot exact approval입니다.
	FCFDAStagingReviewedApproval ReviewedApproval;
};

#if WITH_DEV_AUTOMATION_TESTS
// DAS-P0-05 Automation fixture만 사용하는 deterministic operational fault injection control입니다.
class CARFIGHT_REEDITOR_API FCFDAStagingOpsTestControl
{
public:
	// 모든 test-only Ops fault injection state를 초기화합니다.
	static void Reset();

	// zero-based changed-file write ordinal에서 실제 write 성공 직후 failure를 강제해 rollback을 검증합니다.
	static void ForceSyncFailureAfterWrite(const int32 WriteOrdinal);

	// Normalize/Discover가 실제 사용하는 production mixed operational TypeKey backing authority를 mutation 없이 문자열로 반환합니다.
	static void GetMixedOperationalAllowedTypeKeys(
		TArray<FString>& OutSchemaIds,
		TArray<FString>& OutDataAssetTypeClassPaths);
};
#endif
