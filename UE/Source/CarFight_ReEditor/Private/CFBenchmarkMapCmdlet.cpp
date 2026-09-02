// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBenchmarkMapCmdlet.cpp
// Version: v1.0.0
// Date: 2026-09-01
// Description: CarFight 주행 테스트 전용 M_VehicleBenchmark 최소 골격 생성 구현입니다.
// Changelog:
// - v1.0.0: 500m 직선 TestRoad, VehicleBenchmarkStart, EndPreview, 기본 조명을 빈 월드에 배치하고 /Game/Maps/M_VehicleBenchmark로 저장.
// Migration:
// - existing map overwrite 금지.
// - benchmark runtime source/map binding은 USER 편집 완료 뒤 별도 단계에서 수행.

#include "CFBenchmarkMapCmdlet.h"

#include "FileHelpers.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFBenchmarkMap, Log, All);

namespace CFBenchmarkMap
{
	// 새 benchmark map exact asset path입니다.
	const TCHAR* BenchmarkMapPath = TEXT("/Game/Maps/M_VehicleBenchmark");

	// 테스트 바닥 actor label입니다.
	const TCHAR* TestRoadLabel = TEXT("SM_TestRoad");

	// benchmark spawn actor label입니다.
	const TCHAR* BenchmarkStartLabel = TEXT("VehicleBenchmarkStart");

	// USER가 길이를 늘릴 때 종점 참고로 사용할 actor label입니다.
	const TCHAR* BenchmarkEndLabel = TEXT("BenchmarkEndPreview");

	// UE 기본 Cube는 100cm이므로 X=500은 500m 길이입니다.
	const FVector TestRoadScale(500.0, 40.0, 0.5);

	// 500m 도로의 중심을 +X 250m에 둬 시작점을 X=0 부근에 둡니다.
	const FVector TestRoadLocation(25000.0, 0.0, -25.0);

	// 도로 끝 500m 지점 preview입니다.
	const FVector BenchmarkEndLocation(50000.0, 0.0, 100.0);

	// Benchmark 차량 시작 위치입니다.
	const FVector BenchmarkStartLocation(1000.0, 0.0, 150.0);

	// Process exit code taxonomy입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		AlreadyExists = 111,
		WorldCreateFailed = 112,
		MeshLoadFailed = 113,
		ActorCreateFailed = 114,
		SaveFailed = 115
	};

	// Actor 생성 실패를 한 곳에서 검사합니다.
	template <typename TActor>
	TActor* SpawnRequiredActor(UWorld* World, const FTransform& Transform, const TCHAR* Label)
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TActor* Actor = World->SpawnActor<TActor>(TActor::StaticClass(), Transform, Params);
		if (Actor)
		{
			Actor->SetActorLabel(Label);
		}
		return Actor;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFBenchmarkMapCommandlet::UCFBenchmarkMapCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// 새 M_VehicleBenchmark를 생성하고 기존 Asset이 없을 때만 저장합니다.
int32 UCFBenchmarkMapCommandlet::Main(const FString& Params)
{
	(void)Params;
	using namespace CFBenchmarkMap;

	// Existing benchmark map은 overwrite하지 않습니다.
	FString ExistingFilename;
	if (FPackageName::DoesPackageExist(BenchmarkMapPath, &ExistingFilename))
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_EXISTS path=%s file=%s"), BenchmarkMapPath, *ExistingFilename);
		return static_cast<int32>(EExitCode::AlreadyExists);
	}

	// 새 empty Editor World입니다.
	UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
	if (!World)
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_WORLD_CREATE_FAIL"));
		return static_cast<int32>(EExitCode::WorldCreateFailed);
	}

	// UE 기본 100cm Cube를 평지 바닥으로 사용합니다.
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!CubeMesh)
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_CUBE_LOAD_FAIL"));
		return static_cast<int32>(EExitCode::MeshLoadFailed);
	}

	// 500m x 40m x 0.5m 편집 가능한 TestRoad입니다.
	AStaticMeshActor* TestRoad = SpawnRequiredActor<AStaticMeshActor>(
		World,
		FTransform(FRotator::ZeroRotator, TestRoadLocation, TestRoadScale),
		TestRoadLabel);
	if (!TestRoad || !TestRoad->GetStaticMeshComponent())
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_ROAD_CREATE_FAIL"));
		return static_cast<int32>(EExitCode::ActorCreateFailed);
	}
	TestRoad->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	TestRoad->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);

	// +X forward benchmark 시작점입니다.
	APlayerStart* BenchmarkStart = SpawnRequiredActor<APlayerStart>(
		World,
		FTransform(FRotator::ZeroRotator, BenchmarkStartLocation),
		BenchmarkStartLabel);
	if (!BenchmarkStart)
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_START_CREATE_FAIL"));
		return static_cast<int32>(EExitCode::ActorCreateFailed);
	}
	BenchmarkStart->Tags.AddUnique(TEXT("VehicleBenchmarkStart"));

	// USER가 도로를 늘릴 때 현재 500m 끝 위치를 확인할 preview TargetPoint입니다.
	ATargetPoint* EndPreview = SpawnRequiredActor<ATargetPoint>(
		World,
		FTransform(FRotator::ZeroRotator, BenchmarkEndLocation),
		BenchmarkEndLabel);
	if (!EndPreview)
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_END_CREATE_FAIL"));
		return static_cast<int32>(EExitCode::ActorCreateFailed);
	}
	EndPreview->Tags.AddUnique(TEXT("VehicleBenchmarkEndPreview"));

	// 기본 Directional Light입니다.
	ADirectionalLight* DirectionalLight = SpawnRequiredActor<ADirectionalLight>(
		World,
		FTransform(FRotator(-45.0, -30.0, 0.0), FVector(0.0, 0.0, 1000.0)),
		TEXT("DirectionalLight"));
	if (!DirectionalLight)
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_DIRECTIONAL_LIGHT_FAIL"));
		return static_cast<int32>(EExitCode::ActorCreateFailed);
	}

	// 기본 ambient Sky Light입니다.
	ASkyLight* SkyLight = SpawnRequiredActor<ASkyLight>(
		World,
		FTransform(FRotator::ZeroRotator, FVector(0.0, 0.0, 1000.0)),
		TEXT("SkyLight"));
	if (!SkyLight)
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_SKYLIGHT_FAIL"));
		return static_cast<int32>(EExitCode::ActorCreateFailed);
	}

	// 새 map package로 exact save합니다.
	if (!UEditorLoadingAndSavingUtils::SaveMap(World, BenchmarkMapPath))
	{
		UE_LOG(LogCFBenchmarkMap, Error, TEXT("CF_BENCHMARK_MAP_SAVE_FAIL path=%s"), BenchmarkMapPath);
		return static_cast<int32>(EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFBenchmarkMap,
		Display,
		TEXT("CF_BENCHMARK_MAP_PASS path=%s road_length_m=500 road_width_m=40 start_x_cm=%.1f end_x_cm=%.1f actors=5"),
		BenchmarkMapPath,
		BenchmarkStartLocation.X,
		BenchmarkEndLocation.X);

	return static_cast<int32>(EExitCode::Success);
}
