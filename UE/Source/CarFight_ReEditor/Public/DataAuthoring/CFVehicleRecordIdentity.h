// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRecordIdentity.h
// Version: v1.0.0
// Date: 2026-10-16
// Description: CF-FQ-059 C-1 UI/AI 공용 신규 Vehicle identity R0 규칙.
// Changelog:
// - v1.0.0: 기존 Builder VehicleId/4-field identity 규칙을 UI 없는 순수 helper로 분리하고 AI canonical 입력 검사를 추가.
// Migration:
// - 기존 Builder method 이름과 기본 경로는 보존. AI Preview만 canonical identity를 요구하며 R2 승인/생성/Save 기능은 추가하지 않음.

#pragma once

#include "CoreMinimal.h"

struct FCFVehicleRecordCreateRequest;

/** 기존 Builder와 UI-less AI가 공유하는 신규 Vehicle 기본 자산 identity 규칙입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleRecordIdentity
{
public:
	// VehicleId의 기존 ASCII 영문자·숫자·underscore naming 계약을 검사합니다.
	static bool ValidateVehicleId(const FString& VehicleId, FString& OutError);

	// 유효한 VehicleId로 신규 Definition/Recipe의 기본 패키지·오브젝트 이름 네 개를 만듭니다.
	static bool BuildDefaultIdentity(
		const FString& VehicleId,
		FString& OutDefinitionPackageName,
		FString& OutDefinitionAssetName,
		FString& OutRecipePackageName,
		FString& OutRecipeAssetName,
		FString& OutError);

	// AI 요청이 기본 naming에서 벗어나지 않은 신규 Definition/Recipe identity인지 읽기 전용 검사합니다.
	static bool ValidateCanonicalRequest(const FCFVehicleRecordCreateRequest& Request, FString& OutError);
};
