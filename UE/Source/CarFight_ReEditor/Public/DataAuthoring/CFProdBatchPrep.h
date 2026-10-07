// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdBatchPrep.h
// Version: v1.4.0
// Date: 2026-10-06
// Description: CF-FQ-058 First Production Wave exact34 candidate batch를 persistent mutation 없이 준비하고 immutable Review Package로 고정합니다.
// Changelog:
// - v1.4.0: USER-approved first Production cutover provenance가 same reviewed provider schema identity를 보존하도록 ProviderSchemaFingerprint evidence를 결과에 노출.
// - v1.3.0: approved Review Package에 exact8 typed Production execution manifest를 결속하고 PreparedProduct request/draft evidence를 read-only 결과로 노출.
// - v1.2.0: persisted First Wave resource reuse, Resource Catalog fingerprint, socket capability proof와 ReviewReady 상태를 추가.
// - v1.1.0: Review Package 이전에 아직 선택되지 않은 ResourceReference를 required로 오인하지 않도록 unresolved resource 상태로 명확화.
// - v1.0.0: exact8 Product / exact48 Role Binding / unique Production DA exact34 / finite-ammo exact8 candidate preparation 계약을 최초 추가.
// Migration:
// - 이 계층은 read-only planning/review 전용이며 DataAsset 생성/수정, canonical Workbook 저장, Publication Catalog 갱신, Authority Cutover를 수행하지 않습니다.
// - managed authority cutover 전 First Wave bootstrap/review seed로만 사용합니다. cutover 이후 canonical Workbook이 sole Current authoring authority이며 이 builder를 정상 authoring source로 재사용하지 않습니다.
// - 실제 durable mutation은 기존 Typed Production Provisioning + CFProdTargetBindings 경계만 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentPlanning.h"
#include "DataAuthoring/CFProdProvisioning.h"
#include "EquipmentAuthoring/CFEquipmentBuilderTypes.h"
#include "WeaponAuthoring/CFWeaponGuideTypes.h"

/** First Production Wave candidate가 준비된 현재 checkpoint입니다. */
enum class ECFProdBatchPrepState : uint8
{
	// Candidate topology/balance 자체가 아직 유효하지 않습니다.
	Invalid,

	// exact34 candidate는 준비됐지만 차량 적용에 필요한 Required Resource binding/capability proof가 아직 미완료입니다.
	ResourceBindingPending,

	// Required Resource binding과 persisted socket capability proof까지 완료되어 별도 USER Cutover Acceptance에 제출 가능한 상태입니다.
	ReviewReady
};

/** 실제 Production Provisioning에서 생성할 unique DataAsset target exact1의 read-only candidate identity입니다. */
struct FCFProdCandidateTarget
{
	// Role target의 canonical logical identity입니다.
	FCFContentKey ContentKey;

	// EquipmentPreset / Mount / WeaponDefinition / Projectile / Damage / Ammo 중 exact role입니다.
	FString RoleId;

	// 최초 생성 후 pin할 canonical Production object path입니다.
	FString TargetObjectPath;

	// 여러 Product가 공유하는 target인지 여부입니다.
	bool bShared = false;
};

/** USER-approved Review Package와 exact 결속될 Product exact1의 typed execution 준비 결과입니다. */
struct FCFProdPreparedProduct
{
	// Product-level durable provisioning request입니다.
	FCFProdProvisionRequest Request;

	// Weapon Guide durable backend가 exact target fingerprints와 mutation payload에 사용할 draft입니다.
	FCFWeaponGuideDraft WeaponDraft;

	// Equipment Builder durable backend가 EquipmentPreset fingerprint와 mutation payload에 사용할 draft입니다.
	FCFEquipmentPresetDraft EquipmentDraft;
};

/** exact8 First Production Wave candidate preparation 결과입니다. */
struct FCFProdBatchPrepResult
{
	// Candidate 준비 checkpoint입니다.
	ECFProdBatchPrepState State = ECFProdBatchPrepState::Invalid;

	// exact8 Product + exact34 target payload를 포함한 immutable planning Review Package입니다.
	FCFContentReviewPackage ReviewPackage;

	// 실제 Provisioning에서 사용할 unique DataAsset candidate target exact34입니다.
	TArray<FCFProdCandidateTarget> CandidateTargets;

	// USER 승인 뒤 동일 payload로 실행할 exact8 Product request/draft manifest입니다.
	TArray<FCFProdPreparedProduct> PreparedProducts;

	// exact8 request / exact34 typed target desired payload 전체의 canonical SHA-256입니다.
	FString ExecutionBindingFingerprint;

	// PreparedProducts에서 stable ContentKey로 dedupe한 typed execution target 수입니다.
	int32 UniqueExecutionTargetCount = 0;

	// 차량 적용에 필요한데 resolve되지 않은 Required ResourceReference stable field paths입니다.
	TArray<FString> MissingRequiredResourceBindings;

	// Review Package에 결속된 persisted Resource Catalog canonical fingerprint입니다.
	FString ResourceCatalogFingerprint;

	// Review base snapshot이 사용한 registered Provider schema/ownership canonical fingerprint입니다.
	FString ProviderSchemaFingerprint;

	// Candidate가 stable ResourceId로 사용하는 persisted resource exact count입니다.
	int32 ResolvedResourceCount = 0;

	// Persisted turret mesh에서 직접 증명한 required socket capability exact count입니다.
	int32 SocketCapabilityCheckCount = 0;

	// Review Package post Workbook semantic hash입니다.
	FString CandidateWorkbookSemanticHash;

	// First Production Wave Product 수입니다.
	int32 ProductCount = 0;

	// Product별 6 Role binding 총수입니다.
	int32 RoleBindingCount = 0;

	// Shared topology를 접은 unique 신규 Production DA 수입니다.
	int32 UniqueDataAssetCount = 0;

	// finite-ammo Product별 explicit sortie row 수입니다.
	int32 AmmoLoadCount = 0;

	// 이 preparation 실행이 canonical Workbook을 persistent write하지 않았음을 나타냅니다.
	bool bCanonicalWorkbookWritePerformed = false;

	// 이 preparation 실행이 Production DataAsset mutation을 수행하지 않았음을 나타냅니다.
	bool bProductionMutationPerformed = false;

	// 이 preparation 실행이 authority cutover를 수행하지 않았음을 나타냅니다.
	bool bAuthorityCutoverApplied = false;
};

/**
 * CF-FQ-058 First Production Wave candidate를 기존 Generic Compiler/Planning review 경계 위에서 준비합니다.
 *
 * 이 service는 persistent writer가 아닙니다. exact34 topology / initial balance / canonical naming을
 * transient Workbook candidate로 만들고 immutable Review Package를 생성하는 것까지만 책임집니다.
 */
class FCFProdBatchPrep
{
public:
	/**
	 * WeaponContentRosterPlan current baseline으로 exact8 / exact34 candidate를 준비합니다.
	 *
	 * persisted current Product가 이미 사용하는 mesh/projectile actor/FX를 stable ResourceId로 재사용하고
	 * required turret socket capability까지 확인합니다. 이 proof가 하나라도 빠지면 ResourceBindingPending으로
	 * fail-visible하며 모두 충족된 경우에만 ReviewReady를 반환합니다.
	 */
	static bool PrepareFirstWaveCandidate(
		FCFProdBatchPrepResult& OutResult,
		FString& OutError);

	/**
	 * USER가 승인한 exact Review Package와 PreparedProducts typed execution manifest가 여전히 exact 결속됐는지
	 * durable mutation 전에 read-only 재검증합니다. Managed authority cutover는 이 검증 PASS 없이 execution을 열면 안 됩니다.
	 */
	static bool ValidatePreparedBatchApproval(
		const FCFProdBatchPrepResult& PreparedBatch,
		const FCFContentReviewApproval& Approval,
		FString& OutError);
};
