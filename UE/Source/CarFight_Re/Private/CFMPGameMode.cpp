// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-06-01
// Description: Dedicated Server 테스트용 기본 차량 BP 연결 GameMode 구현입니다.

#include "CFMPGameMode.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFMPGameMode, Log, All);

ACFMPGameMode::ACFMPGameMode()
{
	// [v1.1.0] 기본 GameMode Pawn 흐름이 별도 Pawn을 자동 생성하지 못하도록 비워 둡니다.
	DefaultPawnClass = nullptr;

	// [v1.1.0] 별도 BP GameMode 없이 첫 서버 테스트에서 사용할 기본 차량 BP 클래스입니다.
	static ConstructorHelpers::FClassFinder<APawn> DefaultVehiclePawnClassFinder(TEXT("/Game/CarFight/Vehicles/BP_CFVehiclePawn"));
	if (DefaultVehiclePawnClassFinder.Succeeded())
	{
		VehiclePawnClass = DefaultVehiclePawnClassFinder.Class;
	}
	else
	{
		VehiclePawnClass = nullptr;
		UE_LOG(LogCFMPGameMode, Warning, TEXT("Constructor: Default vehicle Blueprint class was not found at /Game/CarFight/Vehicles/BP_CFVehiclePawn."));
	}

	// [v1.1.0] Dedicated Server 테스트 기본 흐름에서는 로그인 직후 차량을 자동 스폰합니다.
	bSpawnVehicleOnPostLogin = true;

	// [v1.1.0] 첫 fallback 스폰 위치에 사용할 순번입니다.
	SpawnIndex = 0;

	// [v1.1.0] fallback 스폰 위치가 서로 겹치지 않도록 둘 기본 간격입니다.
	SpawnOffsetBetweenPlayers = 600.0f;
}

// 플레이어 로그인 직후 서버 차량 스폰/점유 흐름을 시작합니다.
void ACFMPGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogCFMPGameMode, Log, TEXT("PostLogin: PlayerController=%s"), *GetNameSafe(NewPlayer));

	if (!NewPlayer)
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("PostLogin failed: PlayerController is null."));
		return;
	}

	if (!HasAuthority())
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("PostLogin skipped: GameMode does not have authority."));
		return;
	}

	if (!bSpawnVehicleOnPostLogin)
	{
		UE_LOG(LogCFMPGameMode, Log, TEXT("PostLogin skipped: bSpawnVehicleOnPostLogin is false for %s."), *GetNameSafe(NewPlayer));
		return;
	}

	SpawnVehicleForController(NewPlayer);
}

// 플레이어 로그아웃 시 서버 로그를 남기고 기본 정리를 이어갑니다.
void ACFMPGameMode::Logout(AController* Exiting)
{
	UE_LOG(LogCFMPGameMode, Log, TEXT("Logout: Controller=%s, Pawn=%s"), *GetNameSafe(Exiting), Exiting ? *GetNameSafe(Exiting->GetPawn()) : TEXT("None"));

	Super::Logout(Exiting);
}

// 기본 Pawn 자동 생성 대신 PostLogin 차량 스폰 흐름만 사용하도록 시작 처리를 막습니다.
void ACFMPGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	UE_LOG(LogCFMPGameMode, Log, TEXT("HandleStartingNewPlayer skipped: vehicle spawn is handled by PostLogin. PlayerController=%s"), *GetNameSafe(NewPlayer));
}

// 지정한 컨트롤러를 위한 차량 Pawn을 서버에서 스폰하고 점유시킵니다.
APawn* ACFMPGameMode::SpawnVehicleForController(AController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("SpawnVehicleForController failed: Controller is null."));
		return nullptr;
	}

	if (!HasAuthority())
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("SpawnVehicleForController skipped: GameMode does not have authority. Controller=%s"), *GetNameSafe(PlayerController));
		return nullptr;
	}

	// [v1.0.0] 컨트롤러가 이미 소유 중인 Pawn입니다.
	APawn* ExistingPawn = PlayerController->GetPawn();
	if (ExistingPawn)
	{
		UE_LOG(LogCFMPGameMode, Log, TEXT("SpawnVehicleForController skipped: Controller=%s already owns Pawn=%s."), *GetNameSafe(PlayerController), *GetNameSafe(ExistingPawn));
		return ExistingPawn;
	}

	// [v1.0.0] 이번 스폰에 사용할 차량 Pawn 클래스입니다.
	TSubclassOf<APawn> ResolvedVehiclePawnClass = ResolveVehiclePawnClass();
	if (!ResolvedVehiclePawnClass)
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("SpawnVehicleForController failed: VehiclePawnClass is not set. Controller=%s"), *GetNameSafe(PlayerController));
		return nullptr;
	}

	// [v1.0.0] 이번 차량 스폰에 사용할 위치와 회전입니다.
	FTransform SpawnTransform;
	if (!FindVehicleSpawnTransform(PlayerController, SpawnTransform))
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("SpawnVehicleForController failed: Could not resolve spawn transform. Controller=%s"), *GetNameSafe(PlayerController));
		return nullptr;
	}

	// [v1.0.0] 차량 Pawn을 생성할 월드입니다.
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("SpawnVehicleForController failed: World is null. Controller=%s"), *GetNameSafe(PlayerController));
		return nullptr;
	}

	// [v1.0.0] 스폰 충돌 시 가능한 조정하되 Dedicated Server 테스트 흐름을 유지하기 위한 파라미터입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerController;
	SpawnParameters.Instigator = PlayerController->GetPawn();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// [v1.0.0] 서버에서 생성된 차량 Pawn입니다.
	APawn* SpawnedVehiclePawn = World->SpawnActor<APawn>(ResolvedVehiclePawnClass, SpawnTransform, SpawnParameters);
	if (!SpawnedVehiclePawn)
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("SpawnVehicleForController failed: SpawnActor returned null. Controller=%s, Class=%s"), *GetNameSafe(PlayerController), *GetNameSafe(ResolvedVehiclePawnClass.Get()));
		return nullptr;
	}

	UE_LOG(LogCFMPGameMode, Log, TEXT("SpawnVehicleForController success: Controller=%s, Pawn=%s, Location=%s"), *GetNameSafe(PlayerController), *GetNameSafe(SpawnedVehiclePawn), *SpawnTransform.GetLocation().ToCompactString());

	PlayerController->Possess(SpawnedVehiclePawn);

	if (PlayerController->GetPawn() == SpawnedVehiclePawn)
	{
		UE_LOG(LogCFMPGameMode, Log, TEXT("Possess success: Controller=%s, Pawn=%s"), *GetNameSafe(PlayerController), *GetNameSafe(SpawnedVehiclePawn));
		return SpawnedVehiclePawn;
	}

	UE_LOG(LogCFMPGameMode, Warning, TEXT("Possess failed: Controller=%s, Pawn=%s, CurrentPawn=%s"), *GetNameSafe(PlayerController), *GetNameSafe(SpawnedVehiclePawn), *GetNameSafe(PlayerController->GetPawn()));
	return SpawnedVehiclePawn;
}

// PlayerStart를 우선 사용하고 실패하면 결정적 fallback Transform을 계산합니다.
bool ACFMPGameMode::FindVehicleSpawnTransform(AController* PlayerController, FTransform& OutSpawnTransform)
{
	// [v1.0.0] ChoosePlayerStart가 반환한 시작 지점 액터입니다.
	AActor* PlayerStartActor = PlayerController ? ChoosePlayerStart(PlayerController) : nullptr;
	if (PlayerStartActor)
	{
		OutSpawnTransform = PlayerStartActor->GetActorTransform();
		UE_LOG(LogCFMPGameMode, Log, TEXT("FindVehicleSpawnTransform: Using PlayerStart=%s for Controller=%s."), *GetNameSafe(PlayerStartActor), *GetNameSafe(PlayerController));
		return true;
	}

	// [v1.0.0] 이번 fallback 스폰 위치 계산에 사용할 순번입니다.
	const int32 CurrentSpawnIndex = SpawnIndex;

	// [v1.0.0] 다음 fallback 스폰을 위해 증가시킨 순번입니다.
	SpawnIndex = SpawnIndex + 1;

	// [v1.0.0] SpawnIndex 기반으로 X축 방향에 적용할 거리입니다.
	const float SpawnOffsetX = static_cast<float>(CurrentSpawnIndex) * SpawnOffsetBetweenPlayers;

	// [v1.0.0] PlayerStart가 없을 때 사용하는 결정적 fallback 위치입니다.
	const FVector FallbackLocation(SpawnOffsetX, 0.0f, 300.0f);

	// [v1.0.0] fallback 스폰에서 사용할 기본 회전입니다.
	const FRotator FallbackRotation = FRotator::ZeroRotator;

	OutSpawnTransform = FTransform(FallbackRotation, FallbackLocation);

	UE_LOG(LogCFMPGameMode, Warning, TEXT("FindVehicleSpawnTransform: PlayerStart not found. Using fallback transform. Controller=%s, SpawnIndex=%d, Location=%s"), *GetNameSafe(PlayerController), CurrentSpawnIndex, *FallbackLocation.ToCompactString());
	return true;
}

// 현재 GameMode 설정에서 사용할 차량 Pawn 클래스를 반환합니다.
TSubclassOf<APawn> ACFMPGameMode::ResolveVehiclePawnClass() const
{
	if (!VehiclePawnClass)
	{
		UE_LOG(LogCFMPGameMode, Warning, TEXT("ResolveVehiclePawnClass: VehiclePawnClass is not set and BP_CFVehiclePawn fallback is unavailable."));
		return nullptr;
	}

	return VehiclePawnClass;
}
