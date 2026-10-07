// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdEquipCatalogData.h
// Version: v1.1.0
// Date: 2026-10-06
// Description: CF-FQ-058 CCAS Production Equipment Publication Catalog runtime data contract입니다.
// Changelog:
// - v1.1.0: normal Runtime consumer가 stable Production PrimaryAsset identity로 generated Catalog를 로드하고 published EquipmentPreset exact membership을 조회할 read-only seam을 추가.
// - v1.0.0: stable PrimaryAsset identity, published Product entry, resolved sortie ammo projection과 read-only validation 계약을 최초 추가.
// Migration:
// - RuntimeTestCatalog/Inventory는 이 Catalog의 publication authority가 아닙니다.
// - 이 Asset은 CCAS가 생성·갱신하며 USER 수동 authoring source로 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "Engine/DataAsset.h"
#include "CFProdEquipCatalogData.generated.h"

class UCFEquipmentPresetData;

/** CCAS가 VehicleReady technical proof 뒤 publication하는 Production Equipment Product 한 건입니다. */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFProdEquipCatalogEntry
{
	GENERATED_BODY()

	// Canonical Workbook Product ContentId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="콘텐츠 ID (ContentId)", ToolTip="CCAS Workbook에서 Product를 식별하는 canonical ContentId입니다."))
	FString ContentId;

	// Publication된 exact EquipmentPresetData hard reference입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="장비 프리셋 (EquipmentPresetData)", ToolTip="기술 검증을 통과해 이 Product와 결속된 persisted EquipmentPresetData입니다."))
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

	// Workbook EquipmentPresetAmmoLoads에서 resolve한 기본 출격 탄약 입력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="기본 출격 탄약 (DefaultSortieAmmoLoads)", ToolTip="MaximumLoadableAmmoCount에서 추론하지 않고 Workbook의 명시적 DefaultSortieAmmoCount를 AmmoData와 결속한 기본 출격 탄약 목록입니다."))
	TArray<FCFAmmoSortieLoad> DefaultSortieAmmoLoads;

	// Publication 시점의 transitive required Role graph fingerprint입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="Product Graph 지문 (ProductGraphFingerprint)", ToolTip="Product와 모든 required Role target의 persisted fingerprint를 포함하는 canonical SHA-256 지문입니다."))
	FString ProductGraphFingerprint;
};

/**
 * CCAS가 자동 생성하는 Production Equipment publication membership exact1입니다.
 * AssetManager는 이 Asset 자체의 resolve/cook transport만 담당합니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFProdEquipCatalogData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Stable Production publication PrimaryAsset identity를 반환합니다.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// Stable CFProdEquipCatalog:Production identity로 current generated Production Catalog를 동기 로드합니다.
	static UCFProdEquipCatalogData* LoadProductionCatalog();

	// Generated Catalog 전체 membership과 finite-ammo projection 계약을 검증합니다.
	bool ValidateProductionCatalog(TArray<FText>& OutValidationErrors) const;

	// Canonical ContentId로 published entry를 exact1 조회합니다.
	const FCFProdEquipCatalogEntry* FindEntryByContentId(const FString& ContentId) const;

	// Exact persisted EquipmentPresetData identity로 published entry를 exact1 조회합니다.
	const FCFProdEquipCatalogEntry* FindEntryByEquipmentPreset(const UCFEquipmentPresetData* EquipmentPresetData) const;

	// Generated Catalog schema revision입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="카탈로그 스키마 버전 (CatalogSchemaVersion)", ToolTip="CCAS generated Production publication catalog의 schema revision입니다."))
	int32 CatalogSchemaVersion = 1;

	// Deterministic ContentId 순서의 published Product membership입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="Production 장비 목록 (PublishedEquipment)", ToolTip="ProductGraphVerified와 RuntimeVerified를 통과한 Production Equipment Product만 포함됩니다."))
	TArray<FCFProdEquipCatalogEntry> PublishedEquipment;

	// PublishedEquipment 전체 canonical projection의 SHA-256 fingerprint입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProductionCatalog", meta=(DisplayName="카탈로그 지문 (CatalogFingerprint)", ToolTip="현재 generated publication membership 전체를 식별하는 deterministic SHA-256 지문입니다."))
	FString CatalogFingerprint;
};
