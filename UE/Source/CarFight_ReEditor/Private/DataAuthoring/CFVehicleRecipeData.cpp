// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRecipeData.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: Editor-only Vehicle Recipe lifecycle 구현입니다.
// Changelog:
// - v1.0.0: 일반 Asset 복제 시 새 RecipeId를 발급하는 duplicate identity 계약을 구현.
// Migration:
// - PIE duplicate는 runtime/editor simulation identity 보존을 위해 RecipeId를 재발급하지 않습니다.

#include "DataAuthoring/CFVehicleRecipeData.h"

// Recipe Asset 복제 시 PIE 복제를 제외하고 새 persistent Recipe identity를 부여합니다.
void UCFVehicleRecipeData::PostDuplicate(const bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		RecipeId = FGuid::NewGuid();
	}
}
