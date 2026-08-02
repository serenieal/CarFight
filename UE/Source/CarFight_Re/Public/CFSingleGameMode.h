// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-01
// Description: CarFight 싱글플레이 기본 차량·UI PlayerController GameMode입니다.
// Changelog:
// - v1.3.0: CF-FQ-032 UI-P0-01A ACFPlayerController를 기본 PlayerControllerClass로 연결한다.
// - v1.2.0: GameMode 시작 시 설정 기반 기본 차량 Pawn 클래스를 적용하는 InitGame 경로를 추가한다.
// - v1.1.0: 차량 Blueprint Pawn 경로를 C++ 하드코딩에서 Game 설정 기반 SoftClass로 이전한다.
// - v1.0.0: 멀티플레이 PostLogin 스폰 흐름을 거치지 않는 싱글플레이 기본 차량 GameMode 추가.
// Migration:
// - 차량 Blueprint Pawn 변경은 CFSingleGameMode의 ConfiguredDefaultVehiclePawnClass 설정값으로 관리한다.
// - 기본 PlayerController는 ACFPlayerController이며 기존 Pawn 입력·AimReticle·TargetSelect 생성 경로는 유지한다.
// - 프로젝트 기본 GameMode를 /Script/CarFight_Re.CFSingleGameMode로 변경한다.
// - 기존 CFMPGameMode 기반 Dedicated Server 테스트 스폰 흐름은 싱글플레이 전환에서 제거한다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/SoftObjectPtr.h"
#include "CFSingleGameMode.generated.h"

class APawn;

/**
 * 싱글플레이에서 기본 차량 Pawn을 Unreal 기본 Pawn 생성 흐름으로 스폰하는 GameMode입니다.
 */
UCLASS(Blueprintable, Config=Game)
class CARFIGHT_RE_API ACFSingleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// ACFSingleGameMode의 기본 차량 Pawn 클래스를 초기화합니다.
	ACFSingleGameMode();

	// GameMode 시작 시 설정 기반 기본 차량 Pawn 클래스를 적용합니다.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	// 기본 Pawn으로 사용할 차량 Blueprint 클래스 설정값입니다.
	UPROPERTY(EditDefaultsOnly, Config, Category="CarFight|SingleGameMode", meta=(DisplayName="기본 차량 Pawn 클래스 (ConfiguredDefaultVehiclePawnClass)", ToolTip="싱글플레이 PIE/플레이 시작 시 기본 Pawn으로 사용할 차량 Blueprint 클래스입니다. 비어 있거나 로드 실패하면 C++ CFVehiclePawn을 사용합니다."))
	TSoftClassPtr<APawn> ConfiguredDefaultVehiclePawnClass;

private:
	// 설정값을 읽어 실제 DefaultPawnClass에 넣을 차량 Pawn 클래스를 결정합니다.
	UClass* ResolveDefaultVehiclePawnClass() const;
};
