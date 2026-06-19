// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-06-19
// Description: CarFight 싱글플레이 기본 차량 GameMode입니다.
// Changelog:
// - v1.0.0: 멀티플레이 PostLogin 스폰 흐름을 거치지 않는 싱글플레이 기본 차량 GameMode 추가.
// Migration:
// - 프로젝트 기본 GameMode를 /Script/CarFight_Re.CFSingleGameMode로 변경한다.
// - 기존 CFMPGameMode 기반 Dedicated Server 테스트 스폰 흐름은 싱글플레이 전환에서 제거한다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CFSingleGameMode.generated.h"

/**
 * 싱글플레이에서 기본 차량 Pawn을 Unreal 기본 Pawn 생성 흐름으로 스폰하는 GameMode입니다.
 */
UCLASS(Blueprintable)
class CARFIGHT_RE_API ACFSingleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// ACFSingleGameMode의 기본 차량 Pawn 클래스를 초기화합니다.
	ACFSingleGameMode();
};
