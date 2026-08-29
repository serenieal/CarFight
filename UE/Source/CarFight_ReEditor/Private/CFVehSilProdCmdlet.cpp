// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehSilProdCmdlet.cpp
// Version: v1.0.0
// Date: 2026-08-25
// Description: CF-FQ-039 승인 Vehicle-specific silhouette Source를 Production Texture와 VehicleData identity catalog로 자산화하는 Editor-only Commandlet 구현입니다.
// Changelog:
// - v1.0.0: VT05 Sedan/SUV PNG 2종 import/validation, exact ChassisMesh authority 확인, VehicleData 3종 catalog binding, exact package save와 idempotent validation-only 경로를 추가.
// Migration:
// - 기존 VehicleSilhouette fallback, ArmorCommonPlate/Direction Icon, WBP/Designer Layout은 수정하지 않습니다.
// - 기존 Texture/catalog가 예상 계약과 정확히 일치하면 재저장 없이 PASS하며, 예상과 다르면 overwrite하지 않고 fail-closed합니다.

#include "CFVehSilProdCmdlet.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "CFVehicleData.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureDefines.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UI/CFHUDVisualData.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFVehSilProd, Log, All);

namespace CFVehSilProd
{
	// 하나의 승인 Source PNG와 Production Texture 계약을 묶습니다.
	struct FTextureImportSpec
	{
		// SourceArt/UI/HUD/VT 아래의 정확한 Source 파일명입니다.
		const TCHAR* SourceFileName;

		// /Game/CarFight/UI/HUD/Visual 아래 생성할 Texture2D Asset 이름입니다.
		const TCHAR* AssetName;

		// 승인된 Source 가로 픽셀 크기입니다.
		int32 ExpectedWidth;

		// 승인된 Source 세로 픽셀 크기입니다.
		int32 ExpectedHeight;
	};

	// VehicleData identity와 ChassisMesh authority, 사용할 silhouette Texture를 묶습니다.
	struct FVehicleBindingSpec
	{
		// Runtime catalog key로 사용할 exact VehicleData Object Path입니다.
		const TCHAR* VehicleDataObjectPath;

		// 해당 VehicleData가 반드시 가리켜야 하는 exact ChassisMesh Object Path입니다.
		const TCHAR* ExpectedChassisObjectPath;

		// 이 VehicleData에 연결할 Production silhouette Texture Asset 이름입니다.
		const TCHAR* TextureAssetName;
	};

	// 단계별 실패를 프로세스 종료 코드로 구분합니다.
	enum class EExitCode : int32
	{
		Success = 0,
		PreflightFailed = 81,
		ImportFailed = 82,
		ValidationFailed = 83,
		BindingFailed = 84,
		SaveFailed = 85
	};

	// Production silhouette Texture를 저장할 Content 경로입니다.
	const FString VisualDestinationPath = TEXT("/Game/CarFight/UI/HUD/Visual");

	// Production HUD Visual Data의 exact Object Path입니다.
	const TCHAR* HUDVisualDataObjectPath = TEXT("/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default.DA_CFHUDVisual_Default");

	// 승인된 Sedan/SUV Source와 Production Texture 2종 계약입니다.
	const FTextureImportSpec TextureImportSpecs[] =
	{
		{TEXT("VT05_VehSil_Sedan.png"), TEXT("T_UI_VehSil_Sedan"), 512, 256},
		{TEXT("VT05_VehSil_SUV.png"), TEXT("T_UI_VehSil_SUV"), 512, 256}
	};

	// 현재 CFVehicleData 3종의 exact identity → ChassisMesh → Texture mapping 계약입니다.
	const FVehicleBindingSpec VehicleBindingSpecs[] =
	{
		{
			TEXT("/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan.DA_TestSedan"),
			TEXT("/Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan"),
			TEXT("T_UI_VehSil_Sedan")
		},
		{
			TEXT("/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV.DA_TestSUV"),
			TEXT("/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV"),
			TEXT("T_UI_VehSil_SUV")
		},
		{
			TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"),
			TEXT("/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV"),
			TEXT("T_UI_VehSil_SUV")
		}
	};

	// SourceArt/UI/HUD/VT의 저장소 절대 경로를 계산합니다.
	FString ResolveSourceRoot()
	{
		// UE Project 폴더에서 저장소 root의 VT SourceArt 디렉터리로 이동한 절대 경로입니다.
		FString SourceRoot = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("../SourceArt/UI/HUD/VT")));
		FPaths::NormalizeDirectoryName(SourceRoot);
		return SourceRoot;
	}

	// Asset 이름의 전체 Texture Object Path를 만듭니다.
	FString MakeTextureObjectPath(const TCHAR* AssetName)
	{
		return FString::Printf(TEXT("%s/%s.%s"), *VisualDestinationPath, AssetName, AssetName);
	}

	// Texture가 승인된 512x256 UI Texture 계약을 만족하는지 확인합니다.
	bool ValidateTexture(
		const UTexture2D* Texture,
		const FTextureImportSpec& ImportSpec,
		FString& OutFailureReason)
	{
		if (!Texture)
		{
			OutFailureReason = FString::Printf(TEXT("Vehicle silhouette Texture validation received null Texture: %s"), ImportSpec.AssetName);
			return false;
		}

		// Headless NullRHI import 직후에도 유효한 원본 Source 가로 크기입니다.
		const int32 SourceSizeX = Texture->Source.GetSizeX();

		// Headless NullRHI import 직후에도 유효한 원본 Source 세로 크기입니다.
		const int32 SourceSizeY = Texture->Source.GetSizeY();

		if (SourceSizeX != ImportSpec.ExpectedWidth || SourceSizeY != ImportSpec.ExpectedHeight
			|| Texture->LODGroup != TEXTUREGROUP_UI
			|| Texture->CompressionSettings != TC_EditorIcon
			|| Texture->MipGenSettings != TMGS_NoMipmaps
			|| !Texture->SRGB)
		{
			OutFailureReason = FString::Printf(
				TEXT("Vehicle silhouette Texture contract failed: asset=%s expected=%dx%d actual=%dx%d lod=%d compression=%d mip=%d srgb=%s"),
				*Texture->GetPathName(),
				ImportSpec.ExpectedWidth,
				ImportSpec.ExpectedHeight,
				SourceSizeX,
				SourceSizeY,
				static_cast<int32>(Texture->LODGroup),
				static_cast<int32>(Texture->CompressionSettings),
				static_cast<int32>(Texture->MipGenSettings),
				Texture->SRGB ? TEXT("true") : TEXT("false"));
			return false;
		}
		return true;
	}

	// exact UObject package 하나만 디스크에 저장합니다.
	bool SaveObjectPackage(UObject* AssetObject, FString& OutFailureReason)
	{
		if (!AssetObject)
		{
			OutFailureReason = TEXT("Cannot save null asset object");
			return false;
		}

		// 현재 Asset이 소유된 package입니다.
		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			OutFailureReason = FString::Printf(TEXT("Package is missing for asset: %s"), *AssetObject->GetPathName());
			return false;
		}

		// Long package name을 실제 .uasset 파일 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// Public standalone asset package 저장 옵션입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;

		AssetObject->MarkPackageDirty();
		if (!UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs))
		{
			OutFailureReason = FString::Printf(TEXT("Package save failed: %s"), *PackageFilename);
			return false;
		}
		return true;
	}

	// Source와 현재 Production 상태를 mutation 전에 모두 검증합니다.
	bool ValidatePreflight(
		const FString& SourceRoot,
		UCFHUDVisualData*& OutHUDVisualData,
		TArray<UCFVehicleData*>& OutVehicleDataAssets,
		bool& bOutCatalogAlreadyBound,
		FString& OutFailureReason)
	{
		for (const FTextureImportSpec& ImportSpec : TextureImportSpecs)
		{
			// 현재 승인 Source PNG의 절대 경로입니다.
			const FString SourceFilePath = FPaths::Combine(SourceRoot, ImportSpec.SourceFileName);
			if (!IFileManager::Get().FileExists(*SourceFilePath))
			{
				OutFailureReason = FString::Printf(TEXT("Vehicle silhouette Source is missing: %s"), *SourceFilePath);
				return false;
			}

			// 이미 존재하는 목적 Texture의 exact Object Path입니다.
			const FString TextureObjectPath = MakeTextureObjectPath(ImportSpec.AssetName);

			// partial-state 복구를 허용하기 위해 기존 Texture가 있으면 overwrite하지 않고 정확한 계약만 검증합니다.
			UTexture2D* ExistingTexture = LoadObject<UTexture2D>(nullptr, *TextureObjectPath);
			if (ExistingTexture && !ValidateTexture(ExistingTexture, ImportSpec, OutFailureReason))
			{
				return false;
			}
		}

		OutHUDVisualData = LoadObject<UCFHUDVisualData>(nullptr, HUDVisualDataObjectPath);
		if (!OutHUDVisualData)
		{
			OutFailureReason = FString::Printf(TEXT("HUDVisualData load failed: %s"), HUDVisualDataObjectPath);
			return false;
		}

		OutVehicleDataAssets.Reset();
		for (const FVehicleBindingSpec& BindingSpec : VehicleBindingSpecs)
		{
			// exact catalog key로 사용할 VehicleData Asset입니다.
			UCFVehicleData* VehicleDataAsset = LoadObject<UCFVehicleData>(nullptr, BindingSpec.VehicleDataObjectPath);
			if (!VehicleDataAsset)
			{
				OutFailureReason = FString::Printf(TEXT("VehicleData load failed: %s"), BindingSpec.VehicleDataObjectPath);
				return false;
			}

			if (!VehicleDataAsset->VehicleVisualConfig.ChassisMesh)
			{
				OutFailureReason = FString::Printf(TEXT("VehicleData ChassisMesh is null: %s"), BindingSpec.VehicleDataObjectPath);
				return false;
			}

			// 현재 VehicleData가 실제로 가리키는 ChassisMesh Object Path입니다.
			const FString ActualChassisObjectPath = VehicleDataAsset->VehicleVisualConfig.ChassisMesh->GetPathName();
			if (!ActualChassisObjectPath.Equals(BindingSpec.ExpectedChassisObjectPath, ESearchCase::CaseSensitive))
			{
				OutFailureReason = FString::Printf(
					TEXT("VehicleData ChassisMesh authority mismatch: vehicle=%s expected=%s actual=%s"),
					BindingSpec.VehicleDataObjectPath,
					BindingSpec.ExpectedChassisObjectPath,
					*ActualChassisObjectPath);
				return false;
			}
			OutVehicleDataAssets.Add(VehicleDataAsset);
		}

		bOutCatalogAlreadyBound = false;
		if (OutHUDVisualData->VehicleSilhouettes.IsEmpty())
		{
			return true;
		}

		if (OutHUDVisualData->VehicleSilhouettes.Num() != UE_ARRAY_COUNT(VehicleBindingSpecs))
		{
			OutFailureReason = FString::Printf(
				TEXT("Refusing to replace unexpected VehicleSilhouettes count: %d"),
				OutHUDVisualData->VehicleSilhouettes.Num());
			return false;
		}

		for (int32 BindingIndex = 0; BindingIndex < UE_ARRAY_COUNT(VehicleBindingSpecs); ++BindingIndex)
		{
			// canonical index의 expected binding 계약입니다.
			const FVehicleBindingSpec& BindingSpec = VehicleBindingSpecs[BindingIndex];

			// 현재 DataAsset에 저장된 동일 index catalog entry입니다.
			const FCFHUDVehicleSilhouetteEntry& ExistingEntry = OutHUDVisualData->VehicleSilhouettes[BindingIndex];

			// expected Texture의 exact Object Path입니다.
			const FString ExpectedTextureObjectPath = MakeTextureObjectPath(BindingSpec.TextureAssetName);

			// 현재 catalog에 저장된 VehicleData Soft Object Path입니다.
			const FString ExistingVehiclePath = ExistingEntry.VehicleData.ToSoftObjectPath().ToString();

			// 현재 catalog에 저장된 silhouette Texture Soft Object Path입니다.
			const FString ExistingTexturePath = ExistingEntry.SilhouetteTexture.ToSoftObjectPath().ToString();
			if (!ExistingVehiclePath.Equals(BindingSpec.VehicleDataObjectPath, ESearchCase::CaseSensitive)
				|| !ExistingTexturePath.Equals(ExpectedTextureObjectPath, ESearchCase::CaseSensitive))
			{
				OutFailureReason = FString::Printf(
					TEXT("Refusing to replace unexpected VehicleSilhouettes entry[%d]: vehicle=%s texture=%s"),
					BindingIndex,
					*ExistingVehiclePath,
					*ExistingTexturePath);
				return false;
			}
		}

		bOutCatalogAlreadyBound = true;
		return true;
	}

	// 기존 exact Texture는 validation-only로 재사용하고 없을 때만 신규 import합니다.
	UTexture2D* EnsureProductionTexture(
		IAssetTools& AssetTools,
		const FString& SourceRoot,
		const FTextureImportSpec& ImportSpec,
		bool& bOutImported,
		FString& OutFailureReason)
	{
		// 목적 Production Texture의 exact Object Path입니다.
		const FString TextureObjectPath = MakeTextureObjectPath(ImportSpec.AssetName);

		// partial-state 또는 idempotent rerun에서 재사용 가능한 기존 Texture입니다.
		UTexture2D* ExistingTexture = LoadObject<UTexture2D>(nullptr, *TextureObjectPath);
		if (ExistingTexture)
		{
			bOutImported = false;
			return ValidateTexture(ExistingTexture, ImportSpec, OutFailureReason) ? ExistingTexture : nullptr;
		}

		// 현재 Source PNG의 절대 경로입니다.
		const FString SourceFilePath = FPaths::Combine(SourceRoot, ImportSpec.SourceFileName);

		// 대화상자 없이 신규 Texture를 만들 import task입니다.
		UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
		if (!ImportTask)
		{
			OutFailureReason = FString::Printf(TEXT("AssetImportTask allocation failed: %s"), ImportSpec.AssetName);
			return nullptr;
		}
		ImportTask->Filename = SourceFilePath;
		ImportTask->DestinationPath = VisualDestinationPath;
		ImportTask->DestinationName = ImportSpec.AssetName;
		ImportTask->bAutomated = true;
		ImportTask->bReplaceExisting = false;
		ImportTask->bSave = false;

		// AssetTools에 전달할 단일 import task 목록입니다.
		TArray<UAssetImportTask*> ImportTasks;
		ImportTasks.Add(ImportTask);
		AssetTools.ImportAssetTasks(ImportTasks);

		// import 결과로 생성된 Production Texture입니다.
		UTexture2D* ImportedTexture = LoadObject<UTexture2D>(nullptr, *TextureObjectPath);
		if (!ImportedTexture)
		{
			OutFailureReason = FString::Printf(TEXT("Vehicle silhouette Texture import result is missing: %s"), *TextureObjectPath);
			return nullptr;
		}

		ImportedTexture->Modify();
		ImportedTexture->LODGroup = TEXTUREGROUP_UI;
		ImportedTexture->CompressionSettings = TC_EditorIcon;
		ImportedTexture->MipGenSettings = TMGS_NoMipmaps;
		ImportedTexture->SRGB = true;
		ImportedTexture->MarkPackageDirty();

		if (!ValidateTexture(ImportedTexture, ImportSpec, OutFailureReason))
		{
			return nullptr;
		}

		bOutImported = true;
		return ImportedTexture;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFVehSilProdCommandlet::UCFVehSilProdCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// Sedan/SUV Source 2종을 Production Texture로 보장하고 VehicleData 3종 catalog를 exact mapping으로 저장합니다.
int32 UCFVehSilProdCommandlet::Main(const FString& Params)
{
	(void)Params;

	// 이번 commandlet이 읽을 exact VT Source root입니다.
	const FString SourceRoot = CFVehSilProd::ResolveSourceRoot();

	// preflight에서 로드할 Production HUD Visual Data입니다.
	UCFHUDVisualData* HUDVisualData = nullptr;

	// canonical order로 로드할 VehicleData 3종입니다.
	TArray<UCFVehicleData*> VehicleDataAssets;

	// 기존 catalog가 이미 exact mapping인지 나타냅니다.
	bool bCatalogAlreadyBound = false;

	// 실패 단계의 구체적인 이유를 보존할 문자열입니다.
	FString FailureReason;
	if (!CFVehSilProd::ValidatePreflight(
		SourceRoot,
		HUDVisualData,
		VehicleDataAssets,
		bCatalogAlreadyBound,
		FailureReason))
	{
		UE_LOG(LogCFVehSilProd, Error, TEXT("CF_VEHSIL_PROD_PREFLIGHT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFVehSilProd::EExitCode::PreflightFailed);
	}

	// Source PNG를 Production Texture로 import할 Editor-only AssetTools 인터페이스입니다.
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

	// Texture AssetName으로 최종 Production Texture를 조회할 map입니다.
	TMap<FName, UTexture2D*> TextureByAssetName;

	// 이번 실행에서 실제 신규 import된 Texture 목록입니다.
	TArray<UTexture2D*> ImportedTextures;
	for (const CFVehSilProd::FTextureImportSpec& ImportSpec : CFVehSilProd::TextureImportSpecs)
	{
		// 현재 Texture가 신규 import됐는지 기록합니다.
		bool bImported = false;

		// existing validation 또는 신규 import로 확보한 Production Texture입니다.
		UTexture2D* ProductionTexture = CFVehSilProd::EnsureProductionTexture(
			AssetTools,
			SourceRoot,
			ImportSpec,
			bImported,
			FailureReason);
		if (!ProductionTexture)
		{
			UE_LOG(LogCFVehSilProd, Error, TEXT("CF_VEHSIL_PROD_IMPORT_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFVehSilProd::EExitCode::ImportFailed);
		}

		TextureByAssetName.Add(FName(ImportSpec.AssetName), ProductionTexture);
		if (bImported)
		{
			ImportedTextures.Add(ProductionTexture);
		}
	}

	// 이번 실행에서 HUDVisualData catalog를 신규 설정했는지 나타냅니다.
	bool bCatalogChanged = false;
	if (!bCatalogAlreadyBound)
	{
		if (VehicleDataAssets.Num() != UE_ARRAY_COUNT(CFVehSilProd::VehicleBindingSpecs))
		{
			UE_LOG(LogCFVehSilProd, Error, TEXT("CF_VEHSIL_PROD_BIND_FAIL VehicleData preflight count mismatch"));
			return static_cast<int32>(CFVehSilProd::EExitCode::BindingFailed);
		}

		HUDVisualData->Modify();
		HUDVisualData->VehicleSilhouettes.Reset();
		for (int32 BindingIndex = 0; BindingIndex < UE_ARRAY_COUNT(CFVehSilProd::VehicleBindingSpecs); ++BindingIndex)
		{
			// canonical index의 expected VehicleData→Texture binding 계약입니다.
			const CFVehSilProd::FVehicleBindingSpec& BindingSpec = CFVehSilProd::VehicleBindingSpecs[BindingIndex];

			// current binding에서 사용할 Texture pointer입니다.
			UTexture2D* const* TexturePtr = TextureByAssetName.Find(FName(BindingSpec.TextureAssetName));
			if (!TexturePtr || !*TexturePtr || !VehicleDataAssets.IsValidIndex(BindingIndex) || !VehicleDataAssets[BindingIndex])
			{
				UE_LOG(LogCFVehSilProd, Error, TEXT("CF_VEHSIL_PROD_BIND_FAIL incomplete binding index=%d"), BindingIndex);
				return static_cast<int32>(CFVehSilProd::EExitCode::BindingFailed);
			}

			// DataAsset에 추가할 exact VehicleData identity → silhouette Texture catalog entry입니다.
			FCFHUDVehicleSilhouetteEntry CatalogEntry;
			CatalogEntry.VehicleData = VehicleDataAssets[BindingIndex];
			CatalogEntry.SilhouetteTexture = *TexturePtr;
			HUDVisualData->VehicleSilhouettes.Add(CatalogEntry);
		}
		HUDVisualData->MarkPackageDirty();
		bCatalogChanged = true;
	}

	for (UTexture2D* ImportedTexture : ImportedTextures)
	{
		if (!CFVehSilProd::SaveObjectPackage(ImportedTexture, FailureReason))
		{
			UE_LOG(LogCFVehSilProd, Error, TEXT("CF_VEHSIL_PROD_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFVehSilProd::EExitCode::SaveFailed);
		}
	}

	if (bCatalogChanged && !CFVehSilProd::SaveObjectPackage(HUDVisualData, FailureReason))
	{
		UE_LOG(LogCFVehSilProd, Error, TEXT("CF_VEHSIL_PROD_SAVE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFVehSilProd::EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFVehSilProd,
		Display,
		TEXT("CF_VEHSIL_PROD_PASS imported=%d catalog_entries=%d catalog_changed=%s saved_packages=%d sedan=%s suv=%s armor_mutation=0 widget_mutation=0 fallback_mutation=0"),
		ImportedTextures.Num(),
		HUDVisualData->VehicleSilhouettes.Num(),
		bCatalogChanged ? TEXT("true") : TEXT("false"),
		ImportedTextures.Num() + (bCatalogChanged ? 1 : 0),
		*CFVehSilProd::MakeTextureObjectPath(TEXT("T_UI_VehSil_Sedan")),
		*CFVehSilProd::MakeTextureObjectPath(TEXT("T_UI_VehSil_SUV")));
	return static_cast<int32>(CFVehSilProd::EExitCode::Success);
}
