// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-06-19
// Description: CarFight 싱글플레이 기본 차량 GameMode 구현입니다.
// Changelog:
// - v1.0.0: BP_CFVehiclePawn을 DefaultPawnClass로 연결하는 싱글플레이 GameMode 추가.
// Migration:
// - 프로젝트 설정의 기본 GameMode가 CFSingleGameMode를 가리키면 별도 PostLogin 차량 스폰 로직 없이 차량이 생성된다.

#include "CFSingleGameMode.h"

#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

ACFSingleGameMode::ACFSingleGameMode()
{
	// [v1.0.0] 싱글플레이 기본 Pawn으로 사용할 차량 Blueprint 클래스입니다.
	static ConstructorHelpers::FClassFinder<APawn> VehiclePawnClassFinder(TEXT("/Game/CarFight/Vehicles/BP_CFVehiclePawn"));
	if (VehiclePawnClassFinder.Succeeded())
	{
		DefaultPawnClass = VehiclePawnClassFinder.Class;
	}
}
