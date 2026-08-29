// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehSilSrcCmdlet.cpp
// Version: v1.0.3
// Date: 2026-08-25
// Description: CF-FQ-039 Vehicle-specific silhouette의 exact ChassisMesh LOD0을 Review Source Candidate PNG로 투영하는 Editor-only Commandlet 구현입니다.
// Changelog:
// - v1.0.3: FImageUtils::CompressImageArray의 JPEG 출력과 .png 확장자 불일치를 제거하고 IImageWrapper PNG RGBA 인코딩으로 교체.
// - v1.0.2: UCommandlet::Main에 전달되는 Unreal 공통 실행 플래그를 사용자 입력으로 오인해 exit 71 처리하던 잘못된 unexpected-params preflight를 제거.
// - v1.0.1: UE 5.8 FImageUtils::CompressImageArray 시그니처에 맞춰 PNG byte buffer를 TArray<uint8>로 교정.
// - v1.0.0: Sedan/SUV LOD0 triangle을 Vehicle XY 평면으로 투영하고 +X Front가 화면 왼쪽을 향하도록 2x supersampling RGBA silhouette를 생성.
// Migration:
// - SourceArt/UI/HUD/VT에 신규 VT05 PNG 2개만 기록합니다.
// - 기존 P2/VT03 Source, Production Texture, VehicleSilhouettes catalog, UMG Asset은 읽기/수정/저장하지 않습니다.
// - 목적 PNG가 이미 존재하면 overwrite하지 않고 fail-closed합니다.

#include "CFVehSilSrcCmdlet.h"

#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "RawIndexBuffer.h"
#include "StaticMeshResources.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFVehSilSrc, Log, All);

namespace CFVehSilSrc
{
	// 최종 Review Source Candidate의 가로 픽셀 크기입니다.
	constexpr int32 CanvasWidth = 512;

	// 최종 Review Source Candidate의 세로 픽셀 크기입니다.
	constexpr int32 CanvasHeight = 256;

	// 외곽선 가장자리의 계단 현상을 줄이기 위한 내부 supersampling 배율입니다.
	constexpr int32 SupersampleScale = 2;

	// 최종 Canvas 가장자리에서 차량 실루엣까지 유지할 최소 여백입니다.
	constexpr int32 CanvasPadding = 24;

	// 투영 면적이 사실상 0인 triangle을 제외할 수치 임계값입니다.
	constexpr float DegenerateAreaThreshold = 0.0001f;

	// Commandlet 단계별 실패를 구분할 종료 코드입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		PreflightFailed = 71,
		MeshReadFailed = 72,
		RasterizeFailed = 73,
		WriteFailed = 74
	};

	// 하나의 exact ChassisMesh와 생성할 Review Source Candidate 파일 계약입니다.
	struct FVehicleSilhouetteSpec
	{
		// 로그와 검증에서 사용할 형상 그룹 이름입니다.
		const TCHAR* ShapeName;

		// Source Authority인 exact StaticMesh Object Path입니다.
		const TCHAR* ChassisMeshObjectPath;

		// SourceArt/UI/HUD/VT 아래에 생성할 Review Source Candidate 파일명입니다.
		const TCHAR* OutputFileName;
	};

	// 하나의 생성 결과에서 기술 검증에 필요한 핵심 수치를 보존합니다.
	struct FVehicleSilhouetteResult
	{
		// 읽은 LOD0 정점 수입니다.
		uint32 VertexCount = 0;

		// 읽은 LOD0 triangle 수입니다.
		uint32 TriangleCount = 0;

		// 실제 rasterize에 사용된 비퇴화 triangle 수입니다.
		uint32 RasterizedTriangleCount = 0;

		// 최종 이미지에서 alpha가 0보다 큰 픽셀 수입니다.
		int32 CoveredPixelCount = 0;

		// Vehicle 좌표를 화면 규약으로 바꾼 2D 최소 좌표입니다.
		FVector2f ProjectedMin = FVector2f::ZeroVector;

		// Vehicle 좌표를 화면 규약으로 바꾼 2D 최대 좌표입니다.
		FVector2f ProjectedMax = FVector2f::ZeroVector;
	};

	// 현재 Production VehicleData identity 감사에서 확정된 unique ChassisMesh 두 종입니다.
	const FVehicleSilhouetteSpec VehicleSilhouetteSpecs[] =
	{
		{
			TEXT("Sedan"),
			TEXT("/Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan"),
			TEXT("VT05_VehSil_Sedan.png")
		},
		{
			TEXT("SUV"),
			TEXT("/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV"),
			TEXT("VT05_VehSil_SUV.png")
		}
	};

	// SourceArt/UI/HUD/VT의 저장소 절대 경로를 계산합니다.
	FString ResolveOutputRoot()
	{
		// UE Project 폴더에서 저장소 root의 VT SourceArt 디렉터리로 이동한 절대 경로입니다.
		FString OutputRoot = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("../SourceArt/UI/HUD/VT")));
		FPaths::NormalizeDirectoryName(OutputRoot);
		return OutputRoot;
	}

	// Vehicle 좌표의 X/Y를 HUD top-down left-facing 2D 좌표로 변환합니다.
	FVector2f ProjectVehicleVertex(const FVector3f& VehiclePosition)
	{
		// +X Front -> screen-left, +Y Right -> screen-up 규약을 적용한 2D 좌표입니다.
		return FVector2f(-VehiclePosition.X, -VehiclePosition.Y);
	}

	// 2D triangle의 signed edge 값을 계산합니다.
	float CalculateEdgeValue(const FVector2f& Start, const FVector2f& End, const FVector2f& Point)
	{
		return ((Point.X - Start.X) * (End.Y - Start.Y))
			- ((Point.Y - Start.Y) * (End.X - Start.X));
	}

	// Projected mesh 좌표를 aspect-preserving working canvas 픽셀 좌표로 변환합니다.
	FVector2f TransformToCanvas(
		const FVector2f& ProjectedPosition,
		const FVector2f& ProjectedCenter,
		float PixelScale,
		int32 WorkingWidth,
		int32 WorkingHeight)
	{
		// Working canvas의 중심 X 좌표입니다.
		const float CanvasCenterX = static_cast<float>(WorkingWidth) * 0.5f;

		// Working canvas의 중심 Y 좌표입니다.
		const float CanvasCenterY = static_cast<float>(WorkingHeight) * 0.5f;

		return FVector2f(
			((ProjectedPosition.X - ProjectedCenter.X) * PixelScale) + CanvasCenterX,
			((ProjectedPosition.Y - ProjectedCenter.Y) * PixelScale) + CanvasCenterY);
	}

	// 하나의 projected triangle을 working alpha mask에 채웁니다.
	bool RasterizeTriangle(
		const FVector2f& PointA,
		const FVector2f& PointB,
		const FVector2f& PointC,
		int32 WorkingWidth,
		int32 WorkingHeight,
		TArray<uint8>& WorkingAlpha)
	{
		// triangle의 signed 2D 면적에 비례하는 edge 값입니다.
		const float SignedArea = CalculateEdgeValue(PointA, PointB, PointC);
		if (FMath::Abs(SignedArea) <= DegenerateAreaThreshold)
		{
			return false;
		}

		// triangle bounding box의 최소 X 픽셀입니다.
		const int32 MinPixelX = FMath::Clamp(
			FMath::FloorToInt(FMath::Min3(PointA.X, PointB.X, PointC.X)),
			0,
			WorkingWidth - 1);

		// triangle bounding box의 최대 X 픽셀입니다.
		const int32 MaxPixelX = FMath::Clamp(
			FMath::CeilToInt(FMath::Max3(PointA.X, PointB.X, PointC.X)),
			0,
			WorkingWidth - 1);

		// triangle bounding box의 최소 Y 픽셀입니다.
		const int32 MinPixelY = FMath::Clamp(
			FMath::FloorToInt(FMath::Min3(PointA.Y, PointB.Y, PointC.Y)),
			0,
			WorkingHeight - 1);

		// triangle bounding box의 최대 Y 픽셀입니다.
		const int32 MaxPixelY = FMath::Clamp(
			FMath::CeilToInt(FMath::Max3(PointA.Y, PointB.Y, PointC.Y)),
			0,
			WorkingHeight - 1);

		for (int32 PixelY = MinPixelY; PixelY <= MaxPixelY; ++PixelY)
		{
			for (int32 PixelX = MinPixelX; PixelX <= MaxPixelX; ++PixelX)
			{
				// 현재 픽셀 중심에서 triangle inclusion을 판정할 sample 좌표입니다.
				const FVector2f PixelCenter(
					static_cast<float>(PixelX) + 0.5f,
					static_cast<float>(PixelY) + 0.5f);

				// AB edge 기준 현재 sample의 signed 위치입니다.
				const float EdgeAB = CalculateEdgeValue(PointA, PointB, PixelCenter);

				// BC edge 기준 현재 sample의 signed 위치입니다.
				const float EdgeBC = CalculateEdgeValue(PointB, PointC, PixelCenter);

				// CA edge 기준 현재 sample의 signed 위치입니다.
				const float EdgeCA = CalculateEdgeValue(PointC, PointA, PixelCenter);

				// 세 edge가 모두 음수 방향에 있는지 나타냅니다.
				const bool bHasNegativeEdge = EdgeAB < 0.0f || EdgeBC < 0.0f || EdgeCA < 0.0f;

				// 세 edge가 모두 양수 방향에 있는지 나타냅니다.
				const bool bHasPositiveEdge = EdgeAB > 0.0f || EdgeBC > 0.0f || EdgeCA > 0.0f;

				if (!(bHasNegativeEdge && bHasPositiveEdge))
				{
					// Working mask의 1차원 alpha index입니다.
					const int32 AlphaIndex = (PixelY * WorkingWidth) + PixelX;
					WorkingAlpha[AlphaIndex] = 255;
				}
			}
		}
		return true;
	}

	// 하나의 exact StaticMesh를 512x256 transparent silhouette PNG로 생성합니다.
	bool GenerateSilhouetteSource(
		const FString& OutputRoot,
		const FVehicleSilhouetteSpec& Spec,
		FVehicleSilhouetteResult& OutResult,
		FString& OutFailureReason)
	{
		// 이번 shape group의 exact Source Authority StaticMesh입니다.
		UStaticMesh* ChassisMesh = LoadObject<UStaticMesh>(nullptr, Spec.ChassisMeshObjectPath);
		if (!ChassisMesh)
		{
			OutFailureReason = FString::Printf(TEXT("ChassisMesh load failed: %s"), Spec.ChassisMeshObjectPath);
			return false;
		}

		// StaticMesh가 보유한 현재 render data입니다.
		const FStaticMeshRenderData* MeshRenderData = ChassisMesh->GetRenderData();
		if (!MeshRenderData || MeshRenderData->LODResources.IsEmpty())
		{
			OutFailureReason = FString::Printf(TEXT("LOD0 render data is missing: %s"), Spec.ChassisMeshObjectPath);
			return false;
		}

		// Shape authority에서 읽을 exact LOD0 resource입니다.
		const FStaticMeshLODResources& LODResource = MeshRenderData->LODResources[0];

		// LOD0의 vertex position buffer입니다.
		const FPositionVertexBuffer& PositionBuffer = LODResource.VertexBuffers.PositionVertexBuffer;

		// LOD0의 전체 triangle index view입니다.
		const FIndexArrayView IndexView = LODResource.IndexBuffer.GetArrayView();

		// LOD0 정점 수입니다.
		const uint32 VertexCount = PositionBuffer.GetNumVertices();

		// LOD0 index 수입니다.
		const int32 IndexCount = IndexView.Num();
		if (VertexCount == 0 || IndexCount < 3 || (IndexCount % 3) != 0)
		{
			OutFailureReason = FString::Printf(
				TEXT("Invalid LOD0 geometry: mesh=%s vertices=%u indices=%d"),
				Spec.ChassisMeshObjectPath,
				VertexCount,
				IndexCount);
			return false;
		}

		OutResult.VertexCount = VertexCount;
		OutResult.TriangleCount = static_cast<uint32>(IndexCount / 3);

		// 모든 LOD0 정점을 HUD 2D 규약으로 변환해 보존할 배열입니다.
		TArray<FVector2f> ProjectedVertices;
		ProjectedVertices.SetNumUninitialized(static_cast<int32>(VertexCount));

		// Projected X/Y 최소 범위입니다.
		FVector2f ProjectedMin(FLT_MAX, FLT_MAX);

		// Projected X/Y 최대 범위입니다.
		FVector2f ProjectedMax(-FLT_MAX, -FLT_MAX);

		for (uint32 VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex)
		{
			// 현재 LOD0 정점의 Vehicle local position입니다.
			const FVector3f VehiclePosition = PositionBuffer.VertexPosition(VertexIndex);

			// HUD orientation contract를 적용한 현재 정점의 projected position입니다.
			const FVector2f ProjectedPosition = ProjectVehicleVertex(VehiclePosition);
			ProjectedVertices[static_cast<int32>(VertexIndex)] = ProjectedPosition;
			ProjectedMin.X = FMath::Min(ProjectedMin.X, ProjectedPosition.X);
			ProjectedMin.Y = FMath::Min(ProjectedMin.Y, ProjectedPosition.Y);
			ProjectedMax.X = FMath::Max(ProjectedMax.X, ProjectedPosition.X);
			ProjectedMax.Y = FMath::Max(ProjectedMax.Y, ProjectedPosition.Y);
		}

		OutResult.ProjectedMin = ProjectedMin;
		OutResult.ProjectedMax = ProjectedMax;

		// Projected mesh의 실제 X/Y 크기입니다.
		const FVector2f ProjectedSize = ProjectedMax - ProjectedMin;
		if (ProjectedSize.X <= KINDA_SMALL_NUMBER || ProjectedSize.Y <= KINDA_SMALL_NUMBER)
		{
			OutFailureReason = FString::Printf(
				TEXT("Projected Chassis bounds are invalid: mesh=%s size=(%.3f,%.3f)"),
				Spec.ChassisMeshObjectPath,
				ProjectedSize.X,
				ProjectedSize.Y);
			return false;
		}

		// Supersampling을 적용한 내부 raster 가로 크기입니다.
		const int32 WorkingWidth = CanvasWidth * SupersampleScale;

		// Supersampling을 적용한 내부 raster 세로 크기입니다.
		const int32 WorkingHeight = CanvasHeight * SupersampleScale;

		// Supersampling을 적용한 내부 여백입니다.
		const int32 WorkingPadding = CanvasPadding * SupersampleScale;

		// 차량의 aspect ratio를 유지하면서 canvas에 맞출 X 방향 scale입니다.
		const float ScaleX = static_cast<float>(WorkingWidth - (WorkingPadding * 2)) / ProjectedSize.X;

		// 차량의 aspect ratio를 유지하면서 canvas에 맞출 Y 방향 scale입니다.
		const float ScaleY = static_cast<float>(WorkingHeight - (WorkingPadding * 2)) / ProjectedSize.Y;

		// X/Y 중 더 작은 값을 사용한 uniform pixel scale입니다.
		const float PixelScale = FMath::Min(ScaleX, ScaleY);

		// Projected mesh의 중심입니다.
		const FVector2f ProjectedCenter = (ProjectedMin + ProjectedMax) * 0.5f;

		// Supersampled silhouette coverage를 저장할 working alpha mask입니다.
		TArray<uint8> WorkingAlpha;
		WorkingAlpha.Init(0, WorkingWidth * WorkingHeight);

		// 실제 rasterize된 비퇴화 triangle 개수입니다.
		uint32 RasterizedTriangleCount = 0;

		for (int32 IndexOffset = 0; IndexOffset < IndexCount; IndexOffset += 3)
		{
			// 현재 triangle 첫 번째 vertex index입니다.
			const uint32 VertexIndexA = IndexView[IndexOffset];

			// 현재 triangle 두 번째 vertex index입니다.
			const uint32 VertexIndexB = IndexView[IndexOffset + 1];

			// 현재 triangle 세 번째 vertex index입니다.
			const uint32 VertexIndexC = IndexView[IndexOffset + 2];
			if (VertexIndexA >= VertexCount || VertexIndexB >= VertexCount || VertexIndexC >= VertexCount)
			{
				OutFailureReason = FString::Printf(
					TEXT("LOD0 index out of range: mesh=%s triangle_offset=%d"),
					Spec.ChassisMeshObjectPath,
					IndexOffset);
				return false;
			}

			// 첫 번째 triangle point의 working canvas 좌표입니다.
			const FVector2f CanvasPointA = TransformToCanvas(
				ProjectedVertices[static_cast<int32>(VertexIndexA)],
				ProjectedCenter,
				PixelScale,
				WorkingWidth,
				WorkingHeight);

			// 두 번째 triangle point의 working canvas 좌표입니다.
			const FVector2f CanvasPointB = TransformToCanvas(
				ProjectedVertices[static_cast<int32>(VertexIndexB)],
				ProjectedCenter,
				PixelScale,
				WorkingWidth,
				WorkingHeight);

			// 세 번째 triangle point의 working canvas 좌표입니다.
			const FVector2f CanvasPointC = TransformToCanvas(
				ProjectedVertices[static_cast<int32>(VertexIndexC)],
				ProjectedCenter,
				PixelScale,
				WorkingWidth,
				WorkingHeight);

			if (RasterizeTriangle(
				CanvasPointA,
				CanvasPointB,
				CanvasPointC,
				WorkingWidth,
				WorkingHeight,
				WorkingAlpha))
			{
				++RasterizedTriangleCount;
			}
		}

		OutResult.RasterizedTriangleCount = RasterizedTriangleCount;
		if (RasterizedTriangleCount == 0)
		{
			OutFailureReason = FString::Printf(TEXT("No projected triangle was rasterized: %s"), Spec.ChassisMeshObjectPath);
			return false;
		}

		// 최종 512x256 RGBA 픽셀 배열입니다. RGB는 흰색을 유지하고 alpha만 coverage로 사용합니다.
		TArray<FColor> FinalPixels;
		FinalPixels.SetNumUninitialized(CanvasWidth * CanvasHeight);

		// 최종 이미지에서 alpha가 존재하는 픽셀 수입니다.
		int32 CoveredPixelCount = 0;

		for (int32 PixelY = 0; PixelY < CanvasHeight; ++PixelY)
		{
			for (int32 PixelX = 0; PixelX < CanvasWidth; ++PixelX)
			{
				// 현재 final pixel의 supersample alpha 합계입니다.
				int32 AlphaSum = 0;

				for (int32 SampleY = 0; SampleY < SupersampleScale; ++SampleY)
				{
					for (int32 SampleX = 0; SampleX < SupersampleScale; ++SampleX)
					{
						// Working alpha에서 읽을 X 좌표입니다.
						const int32 WorkingX = (PixelX * SupersampleScale) + SampleX;

						// Working alpha에서 읽을 Y 좌표입니다.
						const int32 WorkingY = (PixelY * SupersampleScale) + SampleY;

						// Working alpha의 1차원 index입니다.
						const int32 WorkingIndex = (WorkingY * WorkingWidth) + WorkingX;
						AlphaSum += static_cast<int32>(WorkingAlpha[WorkingIndex]);
					}
				}

				// Supersample 평균으로 만든 최종 alpha 값입니다.
				const uint8 FinalAlpha = static_cast<uint8>(
					AlphaSum / (SupersampleScale * SupersampleScale));

				// 최종 RGBA 배열의 1차원 index입니다.
				const int32 FinalIndex = (PixelY * CanvasWidth) + PixelX;
				FinalPixels[FinalIndex] = FColor(255, 255, 255, FinalAlpha);
				if (FinalAlpha > 0)
				{
					++CoveredPixelCount;
				}
			}
		}

		OutResult.CoveredPixelCount = CoveredPixelCount;
		if (CoveredPixelCount == 0)
		{
			OutFailureReason = FString::Printf(TEXT("Final silhouette has zero coverage: %s"), Spec.ChassisMeshObjectPath);
			return false;
		}

		// 최종 PNG의 절대 저장 경로입니다.
		const FString OutputFilePath = FPaths::Combine(OutputRoot, Spec.OutputFileName);

		// 실제 PNG RGBA 인코더를 제공하는 Unreal ImageWrapper 모듈입니다.
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

		// PNG 포맷으로 고정된 image wrapper입니다.
		const TSharedPtr<IImageWrapper> PngWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (!PngWrapper.IsValid())
		{
			OutFailureReason = FString::Printf(TEXT("PNG image wrapper creation failed: %s"), *OutputFilePath);
			return false;
		}

		// FColor 메모리의 실제 채널 배열 크기입니다.
		const int64 RawByteCount = static_cast<int64>(FinalPixels.Num()) * static_cast<int64>(sizeof(FColor));
		if (!PngWrapper->SetRaw(
			FinalPixels.GetData(),
			RawByteCount,
			CanvasWidth,
			CanvasHeight,
			ERGBFormat::BGRA,
			8))
		{
			OutFailureReason = FString::Printf(TEXT("PNG raw BGRA setup failed: %s"), *OutputFilePath);
			return false;
		}

		// PNG wrapper가 실제 PNG 포맷으로 압축한 byte 배열입니다.
		const TArray64<uint8>& CompressedPngData = PngWrapper->GetCompressed();
		if (CompressedPngData.IsEmpty())
		{
			OutFailureReason = FString::Printf(TEXT("PNG compression produced empty output: %s"), *OutputFilePath);
			return false;
		}

		if (!FFileHelper::SaveArrayToFile(CompressedPngData, *OutputFilePath))
		{
			OutFailureReason = FString::Printf(TEXT("PNG write failed: %s"), *OutputFilePath);
			return false;
		}

		UE_LOG(
			LogCFVehSilSrc,
			Display,
			TEXT("CF_VEHSIL_SOURCE_SHAPE_PASS shape=%s mesh=%s output=%s vertices=%u triangles=%u rasterized=%u covered_pixels=%d projected_min=(%.3f,%.3f) projected_max=(%.3f,%.3f)"),
			Spec.ShapeName,
			Spec.ChassisMeshObjectPath,
			*OutputFilePath,
			OutResult.VertexCount,
			OutResult.TriangleCount,
			OutResult.RasterizedTriangleCount,
			OutResult.CoveredPixelCount,
			OutResult.ProjectedMin.X,
			OutResult.ProjectedMin.Y,
			OutResult.ProjectedMax.X,
			OutResult.ProjectedMax.Y);
		return true;
	}

	// Output root와 exact 목적 파일의 no-overwrite 조건을 mutation 전에 검증합니다.
	bool ValidatePreflight(const FString& OutputRoot, FString& OutFailureReason)
	{
		if (!IFileManager::Get().DirectoryExists(*OutputRoot))
		{
			OutFailureReason = FString::Printf(TEXT("SourceArt VT output directory is missing: %s"), *OutputRoot);
			return false;
		}

		for (const FVehicleSilhouetteSpec& Spec : VehicleSilhouetteSpecs)
		{
			// 현재 shape group의 목적 PNG 절대 경로입니다.
			const FString OutputFilePath = FPaths::Combine(OutputRoot, Spec.OutputFileName);
			if (IFileManager::Get().FileExists(*OutputFilePath))
			{
				OutFailureReason = FString::Printf(TEXT("Refusing to overwrite existing silhouette Source Candidate: %s"), *OutputFilePath);
				return false;
			}
		}
		return true;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFVehSilSrcCommandlet::UCFVehSilSrcCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// Sedan/SUV exact ChassisMesh를 읽어 SourceArt PNG 2종만 생성합니다.
int32 UCFVehSilSrcCommandlet::Main(const FString& Params)
{
	// 실행 인수는 bounded wrapper가 소유하며 Unreal 공통 commandlet 플래그는 정상 입력으로 허용합니다.
	const FString CommandletParams = Params.TrimStartAndEnd();
	UE_LOG(LogCFVehSilSrc, Verbose, TEXT("CF_VEHSIL_SOURCE_PARAMS %s"), *CommandletParams);

	// Review Source Candidate를 기록할 exact 저장소 경로입니다.
	const FString OutputRoot = CFVehSilSrc::ResolveOutputRoot();

	// 실패 단계의 구체적인 이유를 보존할 문자열입니다.
	FString FailureReason;
	if (!CFVehSilSrc::ValidatePreflight(OutputRoot, FailureReason))
	{
		UE_LOG(LogCFVehSilSrc, Error, TEXT("CF_VEHSIL_SOURCE_PREFLIGHT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFVehSilSrc::EExitCode::PreflightFailed);
	}

	for (const CFVehSilSrc::FVehicleSilhouetteSpec& Spec : CFVehSilSrc::VehicleSilhouetteSpecs)
	{
		// 현재 shape group의 생성 기술 결과입니다.
		CFVehSilSrc::FVehicleSilhouetteResult GenerationResult;
		if (!CFVehSilSrc::GenerateSilhouetteSource(OutputRoot, Spec, GenerationResult, FailureReason))
		{
			UE_LOG(LogCFVehSilSrc, Error, TEXT("CF_VEHSIL_SOURCE_GENERATE_FAIL shape=%s %s"), Spec.ShapeName, *FailureReason);
			return static_cast<int32>(CFVehSilSrc::EExitCode::RasterizeFailed);
		}
	}

	UE_LOG(
		LogCFVehSilSrc,
		Display,
		TEXT("CF_VEHSIL_SOURCE_PASS generated=2 imported=0 bound=0 asset_save=0 source_authority=ChassisMesh catalog_mutation=0"));
	return static_cast<int32>(CFVehSilSrc::EExitCode::Success);
}
