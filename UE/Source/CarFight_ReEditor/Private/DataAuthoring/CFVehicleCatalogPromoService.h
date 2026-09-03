// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleCatalogPromoService.h
// Version: v1.0.0
// Date: 2026-09-02
// Description: CF-FQ-044 VRCP-P0-02 Editor-only Runtime Catalog Promotion owner입니다.
// Changelog:
// - v1.0.0: raw DefaultCatalog resolve, persistent target validation, full validation, exact membership, transactional add/rollback/readback와 no-auto-save 결과 계약을 추가.
// Migration:
// - BuilderVM/BuilderTab/RuntimeApply integration은 이 Service 밖에서 후속 Gate가 소유합니다.
// - SavePackage/SaveAsset/SaveAll을 호출하지 않습니다.

#pragma once

#include "CoreMinimal.h"

class UCFRuntimeTestCatalogData;
class UCFRuntimeTestSettings;
class UCFVehicleData;

/** Runtime Catalog promotion terminal outcome입니다. */
enum class ECFVehicleCatalogPromoOutcome : uint8
{
	Registered,
	AlreadyRegistered,
	InvalidTarget,
	CatalogUnavailable,
	CatalogInvalid,
	MutationFailed
};

/** Runtime Catalog promotion/readback의 typed 결과입니다. */
struct FCFVehicleCatalogPromoResult
{
	// Promotion의 terminal outcome입니다.
	ECFVehicleCatalogPromoOutcome Outcome = ECFVehicleCatalogPromoOutcome::MutationFailed;

	// 호출자가 표시할 수 있는 bounded 진단 메시지입니다.
	FString Message;

	// Result가 가리키는 exact Catalog입니다.
	UCFRuntimeTestCatalogData* Catalog = nullptr;

	// Result가 가리키는 exact VehicleData입니다.
	UCFVehicleData* TargetVehicleData = nullptr;

	// Fresh readback에서 exact target membership이 확인됐는지 여부입니다.
	bool bIsMember = false;

	// Result 생성 시 Catalog package가 dirty인지 여부입니다.
	bool bCatalogPackageDirty = false;

	// 본 Service가 package save를 수행했는지 여부이며 항상 false여야 합니다.
	bool bSavePerformed = false;

	// Registered 또는 AlreadyRegistered인지 반환합니다.
	bool IsSuccess() const
	{
		return Outcome == ECFVehicleCatalogPromoOutcome::Registered
			|| Outcome == ECFVehicleCatalogPromoOutcome::AlreadyRegistered;
	}
};

/** Builder closure가 Runtime Test Catalog allowlist를 안전하게 갱신하도록 하는 Editor-only Service입니다. */
class FCFVehicleCatalogPromoService
{
public:
	// Project Settings의 current DefaultCatalog soft reference를 raw load하며 Catalog validity는 아직 판정하지 않습니다.
	static bool ResolveDefaultRuntimeCatalogRaw(
		UCFRuntimeTestCatalogData*& OutCatalog,
		FCFVehicleCatalogPromoResult& OutFailure);

	// 명시 Settings object의 DefaultCatalog soft reference를 raw load하며 focused automation에서도 동일 계약을 재사용합니다.
	static bool ResolveRuntimeCatalogRaw(
		const UCFRuntimeTestSettings* RuntimeTestSettings,
		UCFRuntimeTestCatalogData*& OutCatalog,
		FCFVehicleCatalogPromoResult& OutFailure);

	// Promotion target이 exact persistent /Game VehicleData인지 fail-closed로 검증합니다.
	static bool ValidatePromotionTarget(
		const UCFVehicleData* TargetVehicleData,
		FString& OutError);

	// Current Catalog 전체 validity를 먼저 확인한 뒤 exact VehicleData membership을 fresh readback합니다.
	static FCFVehicleCatalogPromoResult ReadPromotionMembership(
		UCFVehicleData* TargetVehicleData,
		UCFRuntimeTestCatalogData* RuntimeCatalog);

	// Project Settings의 DefaultCatalog로 exact persistent VehicleData를 idempotent promotion합니다.
	static FCFVehicleCatalogPromoResult PromoteVehicleToRuntimeCatalog(
		UCFVehicleData* TargetVehicleData);

	// 이미 resolve된 Catalog에 exact persistent VehicleData를 idempotent promotion합니다.
	static FCFVehicleCatalogPromoResult PromoteVehicleToCatalog(
		UCFVehicleData* TargetVehicleData,
		UCFRuntimeTestCatalogData* RuntimeCatalog);

	// Fresh result를 USER-facing 한 줄 상태 문자열로 변환합니다.
	static FString BuildPromotionStatus(const FCFVehicleCatalogPromoResult& Result);
};
