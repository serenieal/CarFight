// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-06-01
// Description: Dedicated Server 테스트용 기본 차량 BP 연결 GameMode입니다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "CFMPGameMode.generated.h"

/**
 * Dedicated Server에서 접속한 플레이어에게 차량 Pawn을 스폰하고 점유시키는 최소 GameMode입니다.
 */
UCLASS(Blueprintable)
class CARFIGHT_RE_API ACFMPGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// ACFMPGameMode의 기본값을 초기화합니다.
	ACFMPGameMode();

	// 플레이어 로그인 직후 서버 차량 스폰/점유 흐름을 시작합니다.
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 플레이어 로그아웃 시 서버 로그를 남기고 기본 정리를 이어갑니다.
	virtual void Logout(AController* Exiting) override;

protected:
	// 기본 Pawn 자동 생성 대신 PostLogin 차량 스폰 흐름만 사용하도록 시작 처리를 막습니다.
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// 로그인한 컨트롤러가 사용할 차량 Pawn 클래스입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Multiplayer|Spawn", meta=(DisplayName="차량 Pawn 클래스 (VehiclePawnClass)", ToolTip="로그인한 플레이어에게 서버가 스폰하고 점유시킬 차량 Pawn 클래스입니다. 비어 있으면 기본 BP_CFVehiclePawn fallback을 시도하고, 실패하면 경고 로그만 남깁니다."))
	TSubclassOf<APawn> VehiclePawnClass;

	// PostLogin에서 자동 차량 스폰을 실행할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Multiplayer|Spawn", meta=(DisplayName="PostLogin 차량 자동 스폰 (bSpawnVehicleOnPostLogin)", ToolTip="True이면 플레이어가 로그인한 직후 서버가 차량 Pawn을 스폰하고 컨트롤러에 점유시킵니다."))
	bool bSpawnVehicleOnPostLogin;

	// PlayerStart가 없을 때 fallback 위치 계산에 사용할 다음 스폰 순번입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Multiplayer|Spawn", meta=(DisplayName="스폰 순번 (SpawnIndex)", ToolTip="PlayerStart를 찾지 못했을 때 fallback 위치를 계산하는 데 사용하는 서버 전용 순번입니다."))
	int32 SpawnIndex;

	// fallback 스폰 위치에서 플레이어 사이에 둘 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Multiplayer|Spawn", meta=(ClampMin="0.0", DisplayName="플레이어 간 스폰 간격 (SpawnOffsetBetweenPlayers)", ToolTip="PlayerStart가 없을 때 각 플레이어 차량을 X축 방향으로 떨어뜨릴 거리입니다."))
	float SpawnOffsetBetweenPlayers;

	// 지정한 컨트롤러를 위한 차량 Pawn을 서버에서 스폰하고 점유시킵니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Multiplayer|Spawn", meta=(ToolTip="지정한 컨트롤러가 아직 Pawn을 갖고 있지 않으면 서버에서 차량 Pawn을 스폰하고 점유시킵니다."))
	APawn* SpawnVehicleForController(AController* PlayerController);

	// PlayerStart를 우선 사용하고 실패하면 결정적 fallback Transform을 계산합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Multiplayer|Spawn", meta=(ToolTip="차량 스폰에 사용할 Transform을 찾습니다. PlayerStart를 우선 사용하고 없으면 SpawnIndex 기반 fallback 위치를 사용합니다."))
	bool FindVehicleSpawnTransform(AController* PlayerController, FTransform& OutSpawnTransform);

	// 현재 GameMode 설정에서 사용할 차량 Pawn 클래스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Multiplayer|Spawn", meta=(ToolTip="현재 GameMode에 설정된 차량 Pawn 클래스를 반환합니다. 비어 있으면 nullptr를 반환합니다."))
	TSubclassOf<APawn> ResolveVehiclePawnClass() const;
};
