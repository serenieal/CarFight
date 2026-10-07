// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdProvisioning.h
// Version: v1.2.0
// Date: 2026-10-06
// Description: CF-FQ-058 Typed Production Provisioning, Product Recovery, ammo projection,
// impact withdrawal와 Production Publication Bridge 계약입니다.
// Changelog:
// - v1.2.0: approved ReviewPackage/BaseSnapshot/ResourceCatalog/ExecutionBinding identity를 Product request/transaction에 durable 결속하고 First Wave CreateNew collision stale guard contract를 추가.
// - v1.1.0: F.1 shared single-writer binding mode와 F.2 multi-Product impact closure execution/revalidation report 계약을 추가.
// - v1.0.1: durable target evidence에 TargetObjectPath를 결속하고 impacted publication evidence를 composite Product ContentKey로 보강.
// - v1.0.0: provider-owned EquipmentPresetAmmoLoads schema/projection, Product transaction state,
// typed-backend adapter seam, durable recovery store, impact closure, graph fingerprint,
// runtime proof와 publication bridge 계약을 최초 추가.
// Migration:
// - 이 계층은 새 generic DA writer가 아닙니다. ICFProdTypedTargetAdapter 구현은 기존 typed Provider/Weapon Guide/Equipment Builder/CFDADurableCore만 호출해야 합니다.
// - Runtime proof는 FCFRuntimeEquipApplyService를 재사용하며 별도 Runtime Apply subsystem을 만들지 않습니다.
// - Publication membership graph는 기존 FCFContentDependencyEdge를 입력으로만 소비하며 별도 graph authority를 소유하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFProdEquipCatalogData.h"
#include "DataAuthoring/CFContentTypes.h"

class ACFVehiclePawn;
class UCFAmmoData;
class UCFEquipmentPresetData;

/** Product RoleBinding이 persisted target에 갖는 Production authoring 관계입니다. */
enum class ECFProdBindingMode : uint8
{
	CreateNew,
	CreateNewShared,
	BindShared
};

/** Product-level Production provisioning transaction 상태입니다. */
enum class ECFProdTxnState : uint8
{
	Prepared,
	VisibilityWithdrawn,
	ChildAssetsPersisted,
	ProductGraphVerified,
	RuntimeVerified,
	Published,
	VehicleReady,
	BlockedBeforeMutation,
	RecoveryRequired
};

/** Workbook EquipmentPresetAmmoLoads child row의 provider-owned canonical projection입니다. */
struct FCFProdAmmoLoadRow
{
	// Parent EquipmentPreset/Product ContentId입니다.
	FString ProductContentId;

	// RoleBindings가 resolve할 Ammo Role의 stable ContentId입니다.
	FString AmmoRoleContentId;

	// MaximumLoadableAmmoCount에서 유도하지 않는 explicit 기본 출격 탄약 수량입니다.
	int32 DefaultSortieAmmoCount = 0;
};

/** Product transaction에서 생성/갱신할 typed persisted target 하나입니다. */
struct FCFProdProvisionTarget
{
	// Target의 canonical logical identity입니다.
	FCFContentKey ContentKey;

	// Product composition 안에서 target을 식별하는 stable RoleId입니다.
	FString RoleId;

	// Existing typed backend가 실제 mutation할 canonical UObject path입니다.
	FString TargetObjectPath;

	// 동일 transaction이 요구하는 persisted desired semantic fingerprint입니다.
	FString DesiredFingerprint;

	// ProductGraphFingerprint에 포함되는 required Role인지 여부입니다.
	bool bRequired = true;

	// Product와 target 사이의 CreateNew/CreateNewShared/BindShared authoring 관계입니다.
	ECFProdBindingMode BindingMode = ECFProdBindingMode::CreateNew;

	// 여러 Product가 공유하는 canonical authored target인지 여부입니다. Shared binding mode와 exact 일치해야 합니다.
	bool bShared = false;

	// Review 시점에 반드시 absent였어야 하는 신규 Production target인지 여부입니다. True이면 unexpected-existing drift를 mutation 전에 차단합니다.
	bool bRequireAbsentAtReview = false;

	// Dependency-first deterministic provisioning order입니다. 낮은 값부터 먼저 실행합니다.
	int32 DependencyOrder = 0;
};

/** Product transaction의 target별 durable evidence입니다. */
struct FCFProdTargetEvidence
{
	// Target canonical logical identity입니다.
	FCFContentKey ContentKey;

	// 동일 transaction이 결속된 exact persisted target object path입니다.
	FString TargetObjectPath;

	// 동일 transaction의 expected desired fingerprint입니다.
	FString DesiredFingerprint;

	// Review 당시 absent requirement를 durable resume identity에 보존합니다.
	bool bRequireAbsentAtReview = false;

	// 마지막 durable readback에서 확인한 persisted fingerprint입니다.
	FString PersistedFingerprint;

	// 이 transaction이 target persistent mutation을 실제 수행했는지 여부입니다.
	bool bMutationPerformed = false;
};

/** Product-level durable provisioning/recovery transaction입니다. */
struct FCFProdTransaction
{
	// Stable local transaction identity입니다.
	FString TransactionId;

	// Provisioning 대상 Product ContentKey입니다.
	FCFContentKey ProductKey;

	// USER가 승인한 exact immutable Review Package fingerprint입니다.
	FString ReviewPackageFingerprint;

	// Review의 base whole-Catalog snapshot fingerprint입니다.
	FString BaseCatalogSnapshotFingerprint;

	// Review가 사용한 persisted Resource Catalog/capability fingerprint입니다.
	FString ResourceCatalogFingerprint;

	// Review Package가 포함하는 typed execution manifest fingerprint입니다.
	FString ExecutionBindingFingerprint;

	// Review가 결속된 frozen Workbook semantic hash입니다.
	FString WorkbookSemanticHash;

	// RoleBindings exact topology를 식별하는 canonical fingerprint입니다.
	FString RoleTopologyFingerprint;

	// 현재 Product-level transaction 상태입니다.
	ECFProdTxnState State = ECFProdTxnState::Prepared;

	// Visibility withdrawal 대상이 된 published Product composite ContentKey 집합입니다. Publication adapter에는 여기서 ContentId만 projection합니다.
	TArray<FCFContentKey> ImpactedProductKeys;

	// Target별 object path/desired/persisted mutation evidence입니다.
	TArray<FCFProdTargetEvidence> Targets;

	// Required transitive Role target fingerprint를 포함하는 Product graph fingerprint입니다.
	FString ProductGraphFingerprint;

	// Fail-closed 또는 recovery 상세입니다.
	FString Diagnostic;
};

/** Product Production provisioning 실행 입력입니다. */
struct FCFProdProvisionRequest
{
	// Stable local Product transaction identity입니다.
	FString TransactionId;

	// Canonical Product logical identity입니다.
	FCFContentKey ProductKey;

	// USER가 승인한 exact immutable Review Package fingerprint입니다.
	FString ReviewPackageFingerprint;

	// Review의 base whole-Catalog snapshot fingerprint입니다.
	FString BaseCatalogSnapshotFingerprint;

	// Review가 사용한 persisted Resource Catalog/capability fingerprint입니다.
	FString ResourceCatalogFingerprint;

	// Review Package에 결속된 typed execution manifest fingerprint입니다.
	FString ExecutionBindingFingerprint;

	// Frozen Workbook semantic hash입니다.
	FString WorkbookSemanticHash;

	// Exact RoleBindings topology fingerprint입니다.
	FString RoleTopologyFingerprint;

	// Persisted EquipmentPresetData canonical object path입니다.
	FString EquipmentPresetObjectPath;

	// Dependency-first provisioning 대상 typed target set입니다.
	TArray<FCFProdProvisionTarget> Targets;

	// Provider-owned EquipmentPresetAmmoLoads projection입니다.
	TArray<FCFProdAmmoLoadRow> AmmoLoadRows;

	// Existing compiler가 소유하는 dependency graph입니다.
	TArray<FCFContentDependencyEdge> DependencyEdges;
};

/**
 * 기존 typed writer를 Product orchestration에 연결하는 adapter seam입니다.
 * 구현은 새 writer를 만들지 않고 기존 typed Provider/Guide/Builder/CFDADurableCore에 위임해야 합니다.
 */
class ICFProdTypedTargetAdapter
{
public:
	// Polymorphic adapter를 안전하게 정리합니다.
	virtual ~ICFProdTypedTargetAdapter() = default;

	// Target의 fresh persisted 존재/fingerprint를 mutation 없이 읽습니다.
	virtual bool ReadPersistedFingerprint(
		const FCFProdProvisionTarget& Target,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError) const = 0;

	// Reviewed target을 기존 typed durable backend로 적용하고 fresh persisted fingerprint를 반환합니다.
	virtual bool ApplyReviewedTarget(
		const FCFProdProvisionTarget& Target,
		FString& OutPersistedFingerprint,
		FString& OutError) = 0;
};

/** Product transaction durable persistence boundary입니다. */
class ICFProdTransactionStore
{
public:
	// Polymorphic transaction store를 안전하게 정리합니다.
	virtual ~ICFProdTransactionStore() = default;

	// Exact transaction evidence를 durable 저장합니다.
	virtual bool SaveTransaction(
		const FCFProdTransaction& Transaction,
		FString& OutError) = 0;

	// Exact TransactionId의 durable evidence를 읽고 존재 여부를 분리해 반환합니다.
	virtual bool LoadTransaction(
		const FString& TransactionId,
		bool& bOutFound,
		FCFProdTransaction& OutTransaction,
		FString& OutError) const = 0;
};

/** UE/Saved/CCAS/ProductionTransactions를 사용하는 Product transaction durable store입니다. */
class FCFProdTransactionFileStore final : public ICFProdTransactionStore
{
public:
	// Exact transaction evidence를 atomic sibling replace 방식으로 저장합니다.
	virtual bool SaveTransaction(
		const FCFProdTransaction& Transaction,
		FString& OutError) override;

	// Exact TransactionId JSON을 읽고 schema/filename identity까지 검증합니다.
	virtual bool LoadTransaction(
		const FString& TransactionId,
		bool& bOutFound,
		FCFProdTransaction& OutTransaction,
		FString& OutError) const override;
};

/** Generated Production Publication Catalog persistence boundary입니다. */
class ICFProdPublicationStore
{
public:
	// Polymorphic publication store를 안전하게 정리합니다.
	virtual ~ICFProdPublicationStore() = default;

	// 현재 published Product ContentId exact set을 읽습니다.
	virtual bool ReadPublishedProductIds(
		TArray<FString>& OutProductContentIds,
		FString& OutError) const = 0;

	// Impact closure Product들을 DA mutation 전에 durable Catalog membership에서 제거합니다.
	virtual bool WithdrawProducts(
		const TArray<FString>& ProductContentIds,
		FString& OutError) = 0;

	// RuntimeVerified Product exact1을 generated Catalog에 durable publish합니다.
	virtual bool PublishProduct(
		const FCFProdEquipCatalogEntry& Entry,
		FString& OutError) = 0;

	// Publication 뒤 동일 canonical Product entry를 fresh readback합니다.
	virtual bool ReadPublishedEntry(
		const FString& ProductContentId,
		FCFProdEquipCatalogEntry& OutEntry,
		FString& OutError) const = 0;
};

/** Canonical UCFProdEquipCatalogData exact1을 CFDADurableCore로 갱신하는 publication adapter입니다. */
class FCFProdCatalogStore final : public ICFProdPublicationStore
{
public:
	// Canonical Production Catalog object path를 사용합니다.
	FCFProdCatalogStore();

	// Automation/disposable test에서만 별도 Catalog object path를 명시합니다.
	explicit FCFProdCatalogStore(const FString& InCatalogObjectPath);

	// 현재 published Product ContentId exact set을 읽습니다.
	virtual bool ReadPublishedProductIds(
		TArray<FString>& OutProductContentIds,
		FString& OutError) const override;

	// Impact closure Product들을 durable Catalog update로 withdraw합니다.
	virtual bool WithdrawProducts(
		const TArray<FString>& ProductContentIds,
		FString& OutError) override;

	// RuntimeVerified Product exact1을 durable Catalog create/update로 publish합니다.
	virtual bool PublishProduct(
		const FCFProdEquipCatalogEntry& Entry,
		FString& OutError) override;

	// Publication 뒤 동일 canonical Product entry를 persisted Catalog에서 fresh readback합니다.
	virtual bool ReadPublishedEntry(
		const FString& ProductContentId,
		FCFProdEquipCatalogEntry& OutEntry,
		FString& OutError) const override;

	// 이 store가 소유하는 exact Catalog object path를 반환합니다.
	const FString& GetCatalogObjectPath() const { return CatalogObjectPath; }

private:
	// Canonical 또는 disposable Production Catalog exact object path입니다.
	FString CatalogObjectPath;
};

/** Multi-Product impact closure 실행 한 건의 Product-specific Runtime proof context입니다. */
struct FCFProdProductExecution
{
	// 이 Product에 대한 frozen reviewed provisioning request입니다.
	FCFProdProvisionRequest Request;

	// Pre-publication direct Runtime technical proof 대상 Vehicle입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;

	// Vehicle 안에서 Product를 technical apply할 exact MountProfileId입니다.
	FName TargetMountProfileId = NAME_None;
};

/** Multi-Product impact closure 실행 결과입니다. */
struct FCFProdClosureExecutionReport
{
	// Deterministic Product execution order의 terminal transaction snapshots입니다.
	TArray<FCFProdTransaction> Transactions;

	// VehicleReady까지 통과한 Product composite identities입니다.
	TArray<FCFContentKey> VehicleReadyProductKeys;

	// 실패 또는 recovery 상태로 unpublished 유지된 Product identities입니다.
	TArray<FCFContentKey> RecoveryRequiredProductKeys;

	// Closure-level fail-closed 또는 partial failure 설명입니다.
	FString Diagnostic;
};

/** Pre-publication direct Runtime technical proof boundary입니다. */
class ICFProdRuntimeVerifier
{
public:
	// Polymorphic verifier를 안전하게 정리합니다.
	virtual ~ICFProdRuntimeVerifier() = default;

	// Existing Fitting/Runtime authority로 exact EquipmentPreset + resolved ammo loads를 기술 검증합니다.
	virtual bool VerifyRuntime(
		ACFVehiclePawn* VehiclePawn,
		FName TargetMountProfileId,
		UCFEquipmentPresetData* EquipmentPresetData,
		const TArray<FCFAmmoSortieLoad>& InitialSortieAmmoLoads,
		FString& OutError) = 0;
};

/** FCFRuntimeEquipApplyService direct apply overload를 호출하는 production Runtime verifier입니다. */
class FCFProdRuntimeVerifier final : public ICFProdRuntimeVerifier
{
public:
	// 기존 Runtime Apply service를 통해 technical apply/readback까지 검증합니다.
	virtual bool VerifyRuntime(
		ACFVehiclePawn* VehiclePawn,
		FName TargetMountProfileId,
		UCFEquipmentPresetData* EquipmentPresetData,
		const TArray<FCFAmmoSortieLoad>& InitialSortieAmmoLoads,
		FString& OutError) override;
};

/** Provider-owned Production schema/projection helper입니다. */
class FCFProdWeaponSchema
{
public:
	// EquipmentPresetAmmoLoads child sheet descriptor를 frozen P0-07 schema로 구성합니다.
	static FCFContentSheetDescriptor BuildAmmoLoadSheetDescriptor();

	// EquipmentPreset record의 EquipmentPresetAmmoLoads collection을 typed rows로 projection합니다.
	static bool ProjectAmmoLoadRows(
		const FCFContentRecord& EquipmentPresetRecord,
		TArray<FCFProdAmmoLoadRow>& OutRows,
		FString& OutError);
};

/** Product graph/ammo/impact deterministic helper입니다. */
class FCFProdProvisioning
{
public:
	// Existing dependency graph의 reverse closure에서 현재 published Product만 계산합니다.
	static bool BuildPublishedImpactClosure(
		const FCFProdProvisionRequest& Request,
		const TArray<FCFContentKey>& ChangedContentKeys,
		const TArray<FString>& PublishedProductContentIds,
		TArray<FString>& OutImpactedProductContentIds,
		FString& OutError);

	// Required transitive target persisted fingerprints를 포함하는 ProductGraphFingerprint를 생성합니다.
	static bool BuildProductGraphFingerprint(
		const FCFProdProvisionRequest& Request,
		const TArray<FCFProdTargetEvidence>& TargetEvidence,
		FString& OutFingerprint,
		FString& OutError);

	// Workbook explicit ammo rows와 current fitting을 merge해 Runtime candidate InitialSortieAmmoLoads를 구성합니다.
	static bool BuildRuntimeAmmoLoads(
		const FCFProdProvisionRequest& Request,
		const UCFEquipmentPresetData& EquipmentPresetData,
		const ACFVehiclePawn* VehiclePawn,
		TArray<FCFAmmoSortieLoad>& OutAmmoLoads,
		FString& OutError);

	// Product-level transaction을 withdrawal→typed persist→graph verify→Runtime proof→publish→VehicleReady 순으로 실행/재개합니다.
	static bool ExecuteProductTransaction(
		const FCFProdProvisionRequest& Request,
		ICFProdTypedTargetAdapter& TypedTargetAdapter,
		ICFProdTransactionStore& TransactionStore,
		ICFProdPublicationStore& PublicationStore,
		ICFProdRuntimeVerifier& RuntimeVerifier,
		ACFVehiclePawn* VehiclePawn,
		FName TargetMountProfileId,
		FCFProdTransaction& OutTransaction);

	// Shared single-writer topology를 mutation0 검증하고 impacted published closure 전체를 deterministic하게 재실행합니다.
	static bool ExecuteProductClosure(
		const TArray<FCFProdProductExecution>& ProductExecutions,
		ICFProdTypedTargetAdapter& TypedTargetAdapter,
		ICFProdTransactionStore& TransactionStore,
		ICFProdPublicationStore& PublicationStore,
		ICFProdRuntimeVerifier& RuntimeVerifier,
		FCFProdClosureExecutionReport& OutReport);
};
