// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRecordIdentity.cpp
// Version: v1.0.0
// Date: 2026-10-16
// Description: UI/AI 공용 VehicleId와 신규 생성 위치의 결정적 identity 구현.
// Changelog:
// - v1.0.0: Builder v1.39.2 기존 naming 의미를 동일하게 분리하고 AI만 exact canonical identity 검사.
// Migration:
// - 기존 Slate Advanced 경로 입력/Preview/기존 R2 Create 검증은 변경 없음. AI에게 생성 승인 증표를 제공하지 않음.

#include "DataAuthoring/CFVehicleRecordIdentity.h"

#include "DataAuthoring/CFVehicleUXTypes.h"

// 기존 Builder의 VehicleId 문자열 유효성 검사와 동일한 오류 의미를 반환합니다.
bool FCFVehicleRecordIdentity::ValidateVehicleId(const FString& VehicleId, FString& OutError)
{
	if (VehicleId.IsEmpty())
	{
		OutError = TEXT("Vehicle ID를 입력하세요. 영문자, 숫자, _만 사용할 수 있습니다.");
		return false;
	}

	if (VehicleId != VehicleId.TrimStartAndEnd())
	{
		OutError = TEXT("Vehicle ID 앞뒤에는 공백을 사용할 수 없습니다. 영문자, 숫자, _만 사용하세요.");
		return false;
	}

	for (const TCHAR Character : VehicleId)
	{
		// VehicleId에 허용되는 ASCII 영문자입니다.
		const bool bIsAsciiLetter = (Character >= TEXT('A') && Character <= TEXT('Z'))
			|| (Character >= TEXT('a') && Character <= TEXT('z'));
		// VehicleId에 허용되는 ASCII 숫자입니다.
		const bool bIsAsciiDigit = Character >= TEXT('0') && Character <= TEXT('9');
		if (!bIsAsciiLetter && !bIsAsciiDigit && Character != TEXT('_'))
		{
			OutError = FString::Printf(
				TEXT("Vehicle ID에 사용할 수 없는 문자가 있습니다: '%c'. 영문자, 숫자, _만 사용하세요."),
				Character);
			return false;
		}
	}

	OutError.Reset();
	return true;
}

// 기존 Builder가 제안한 정확한 4-field 신규 자산 경로를 생성합니다.
bool FCFVehicleRecordIdentity::BuildDefaultIdentity(
	const FString& VehicleId,
	FString& OutDefinitionPackageName,
	FString& OutDefinitionAssetName,
	FString& OutRecipePackageName,
	FString& OutRecipeAssetName,
	FString& OutError)
{
	OutDefinitionPackageName.Reset();
	OutDefinitionAssetName.Reset();
	OutRecipePackageName.Reset();
	OutRecipeAssetName.Reset();

	if (!ValidateVehicleId(VehicleId, OutError))
	{
		return false;
	}

	// 프로젝트 신규 Vehicle authoring 자산에만 사용하는 기본 package root입니다.
	const FString AuthoringPackageRoot = TEXT("/Game/CarFight/Data/Authoring/");
	OutDefinitionAssetName = TEXT("DA_Vehicle_") + VehicleId;
	OutRecipeAssetName = TEXT("DA_Recipe_") + VehicleId;
	OutDefinitionPackageName = AuthoringPackageRoot + OutDefinitionAssetName;
	OutRecipePackageName = AuthoringPackageRoot + OutRecipeAssetName;
	OutError.Reset();
	return true;
}

// AI가 제시한 Definition/Recipe 네 identity 값이 기존 Builder 기본 경로와 동일한지 검사합니다.
bool FCFVehicleRecordIdentity::ValidateCanonicalRequest(
	const FCFVehicleRecordCreateRequest& Request,
	FString& OutError)
{
	// Definition 오브젝트 이름에 담긴 차량 ID 후보입니다.
	FString VehicleId = Request.DefinitionAssetName.ToString();
	if (!VehicleId.RemoveFromStart(TEXT("DA_Vehicle_"), ESearchCase::CaseSensitive))
	{
		OutError = TEXT("AI 신규 차량 Definition 이름에는 DA_Vehicle_ 접두사가 필요합니다.");
		return false;
	}

	// ID에서 계산한 기본 Definition package입니다.
	FString ExpectedDefinitionPackageName;
	// ID에서 계산한 기본 Definition asset 이름입니다.
	FString ExpectedDefinitionAssetName;
	// ID에서 계산한 기본 Recipe package입니다.
	FString ExpectedRecipePackageName;
	// ID에서 계산한 기본 Recipe asset 이름입니다.
	FString ExpectedRecipeAssetName;
	if (!BuildDefaultIdentity(
		VehicleId,
		ExpectedDefinitionPackageName,
		ExpectedDefinitionAssetName,
		ExpectedRecipePackageName,
		ExpectedRecipeAssetName,
		OutError))
	{
		return false;
	}

	if (!Request.DefinitionPackageName.Equals(ExpectedDefinitionPackageName, ESearchCase::CaseSensitive)
		|| !Request.RecipePackageName.Equals(ExpectedRecipePackageName, ESearchCase::CaseSensitive)
		|| !Request.DefinitionAssetName.ToString().Equals(ExpectedDefinitionAssetName, ESearchCase::CaseSensitive)
		|| !Request.RecipeAssetName.ToString().Equals(ExpectedRecipeAssetName, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("AI 신규 차량은 Builder 기본 Definition/Recipe package 및 object identity가 정확히 일치해야 합니다.");
		return false;
	}

	OutError.Reset();
	return true;
}
