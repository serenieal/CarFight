// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringToolset.h
// Version: v1.1.0
// Date: 2026-10-20
// Description: CarFight 신규 차량 R0 canonical identity와 증명된 관리형 R2 1쌍 생성 UE Toolset.
// Changelog:
// - v1.1.0: Add verified one-use Bridge R2 Toolset receiver while preserving the original mutation0 identity tool.
// - v1.0.0: 기존 Builder/AI 공용 VehicleId canonical naming의 읽기 전용 Toolset 진입점 추가.
// Migration:
// - v1.1.0: R2 생성 Toolset 메서드는 노출하되 실제 생성은 Bridge의 소비형 실행 증명 확인 후에만 허용. Save/Apply는 계속 수행하지 않음.
// - CarFight_ReEditor Editor 모듈에서만 사용하며 Runtime/Blueprint/기존 Writer 동작은 불변.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/ToolsetDefinition.h"

#include "CFVehicleAuthoringToolset.generated.h"

/** 기존 CarFight 차량 생성 경로를 읽기 전용으로 계산한 결과입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleIdentityToolResult
{
	GENERATED_BODY()

	// VehicleId가 기존 Builder의 canonical naming 규칙을 통과했는지 표시합니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	bool bValid = false;

	// 신규 Definition이 사용할 정확한 패키지 경로입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString DefinitionPackageName;

	// 신규 Definition 오브젝트 이름입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString DefinitionAssetName;

	// 신규 Recipe가 사용할 정확한 패키지 경로입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString RecipePackageName;

	// 신규 Recipe 오브젝트 이름입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString RecipeAssetName;

	// VehicleId가 잘못되었을 때 기존 검증기가 반환한 오류입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString ErrorMessage;
};

/** 인증된 관리형 쓰기 실행 결과를 반환하며 토큰·원시 입력을 노출하지 않습니다. */
USTRUCT(BlueprintType)
struct FCFVehicleCreateToolResult
{
	GENERATED_BODY()

	// 생성 작업 성공 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	bool bSucceeded = false;

	// 실패한 경우의 안정적인 오류 분류입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString ErrorCode;

	// 성공 시 실제 생성된 Definition 오브젝트 경로입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString DefinitionPath;

	// 성공 시 실제 생성된 Recipe 오브젝트 경로입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	FString RecipePath;

	// 이 R2 호출은 Save를 수행하지 않습니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleAuthoring")
	bool bSavePerformed = false;
};

/** 기존 R0 조회와 별도의 인증된 관리형 AI R2 신규 차량 생성 경로를 제공합니다. */
UCLASS(BlueprintType, Hidden)
class UCFVehicleAuthoringToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:
	/** 입력 VehicleId로부터 기존 Builder의 Definition·Recipe 기본 이름 4개를 계산합니다. UObject 생성, 변경, Save는 수행하지 않습니다.
	 * @param VehicleId 기존 Builder가 사용하는 ASCII 영문·숫자·underscore 차량 식별자
	 * @return VehicleId 검증 결과, Definition/Recipe 패키지·오브젝트 경로 및 진단 문자열
	 */
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleAuthoring", meta=(AICallable))
	static FCFVehicleIdentityToolResult PreviewVehicleRecordIdentity(const FString& VehicleId);

	/** Host가 승인한 신규 차량 1쌍 요청을 검증한 1회용 실행 증명으로 생성합니다.
	 * @param request_json 정확히 {"VehicleId":"..."} 형태의 새 차량 생성 JSON
	 * @param gopymcp_execution_proof GoPyMCP Bridge가 실행 직전에 주입하는 소비형 증명
	 * @return 생성 결과 및 미저장 상태; 실패 시 자산 생성 권한 없음
	 */
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleAuthoring", meta=(AICallable))
	static FCFVehicleCreateToolResult CreateNewVehicleRecords(
		const FString& request_json, const FString& gopymcp_execution_proof);
};
