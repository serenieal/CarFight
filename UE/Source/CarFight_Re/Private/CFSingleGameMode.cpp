// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-01
// Description: CarFight 싱글플레이 기본 차량·UI PlayerController GameMode 구현입니다.
// Changelog:
// - v1.3.0: CF-FQ-032 UI-P0-01A 기본 PlayerControllerClass를 ACFPlayerController로 설정한다.
// - v1.2.0: Blueprint Pawn 에셋 경로 하드코딩을 제거하고 ConfiguredDefaultVehiclePawnClass 설정 기반 로드로 전환한다.
// - v1.1.0: BP_CFVehiclePawn 폴더 이동에 맞춰 DefaultPawnClass 로드 경로를 /Game/CarFight/Vehicles/Blueprints로 갱신하고, 로드 실패 시 C++ 차량 Pawn으로 대체한다.
// - v1.0.0: BP_CFVehiclePawn을 DefaultPawnClass로 연결하는 싱글플레이 GameMode 추가.
// Migration:
// - C++ 코드에는 Blueprint 에셋 경로를 넣지 않는다. 차량 Pawn Blueprint 변경은 DefaultGame.ini 또는 에디터 Class Defaults에서 ConfiguredDefaultVehiclePawnClass를 수정한다.
// - ACFPlayerController는 UI Root를 직접 만들지 않고 LocalPlayer UCFUISubsystem 등록만 수행한다.
// - BP_CFVehiclePawn은 /Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn 경로에 저장한다.
// - 프로젝트 설정의 기본 GameMode가 CFSingleGameMode를 가리키면 별도 PostLogin 차량 스폰 로직 없이 차량이 생성된다.

#include "CFSingleGameMode.h"

#include "CFPlayerController.h"
#include "CFVehiclePawn.h"
#include "GameFramework/Pawn.h"

ACFSingleGameMode::ACFSingleGameMode()
{
	// [v1.2.0] 설정 로드 전에도 사용할 수 있는 C++ 기본 차량 Pawn 클래스입니다.
	DefaultPawnClass = ACFVehiclePawn::StaticClass();

	// [v1.3.0] Pawn 수명과 독립된 UI 공통 입력과 LocalPlayer UI 연결을 소유할 기본 Controller 클래스입니다.
	PlayerControllerClass = ACFPlayerController::StaticClass();
}

void ACFSingleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// [v1.2.0] 실제 플레이 시작 전에 설정 기반 기본 차량 Pawn 클래스를 확정합니다.
	DefaultPawnClass = ResolveDefaultVehiclePawnClass();
}

UClass* ACFSingleGameMode::ResolveDefaultVehiclePawnClass() const
{
	// [v1.2.0] 설정에 지정된 차량 Blueprint Pawn 클래스입니다.
	UClass* LoadedVehiclePawnClass = ConfiguredDefaultVehiclePawnClass.LoadSynchronous();
	if (LoadedVehiclePawnClass)
	{
		return LoadedVehiclePawnClass;
	}

	return ACFVehiclePawn::StaticClass();
}
