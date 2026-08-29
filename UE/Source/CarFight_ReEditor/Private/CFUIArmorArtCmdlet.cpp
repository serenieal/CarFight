// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFUIArmorArtCmdlet.cpp
// Version: v1.1.0
// Date: 2026-08-25
// Description: CF-FQ-039 modular Armor Production Texture를 slice별로 안전하게 import/binding하는 Editor Commandlet 구현입니다.
// Changelog:
// - v1.1.0: 기존 2-icon one-shot 경로를 보존하고 -PlateOnly에서 VT04_ArmorPlate → T_UI_ArmorPlate → ArmorCommonPlate exact 2-package save를 추가. 기존 Icon Texture/binding은 validation-only.
// - v1.0.1: Headless NullRHI import 직후 PlatformData 기반 GetSizeX/Y가 0을 반환하는 경우를 피하도록 immutable Source dimension(Texture->Source)을 검증 기준으로 교정.
// - v1.0.0: VT04 Arrow/Chevron2 Source PNG preflight → no-overwrite Texture import → UE 5.8 UI Texture 설정 → HUDVisualData 두 Soft Reference binding → exact package save를 추가.
// Migration:
// - 기본 모드는 기존 VT04 Arrow/Chevron2 두 icon만 읽고 기존 one-shot no-overwrite 계약을 유지합니다.
// - -PlateOnly는 VT04_ArmorPlate.png만 신규 Source로 읽고 기존 두 Icon Texture/binding은 validation-only로 취급합니다.
// - WBP_CFArmorSector, WBP_CFArmorBodyMap, VehiclePanel, legacy ArmorPlates와 VehicleSilhouette catalog는 수정하지 않습니다.
// - 각 slice의 신규 목적 Texture가 이미 존재하면 저장 전에 중단합니다.

#include "CFUIArmorArtCmdlet.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureDefines.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UI/CFHUDVisualData.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFUIArmorArt, Log, All);

namespace CFUIArmorArt
{
	// 하나의 Source PNG와 생성할 Production Texture Asset 계약을 묶습니다.
	struct FArmorTextureImportSpec
	{
		// SourceArt/UI/HUD/VT 아래의 정확한 Source 파일명입니다.
		const TCHAR* SourceFileName;

		// /Game/CarFight/UI/HUD/Visual 아래 생성할 Texture2D Asset 이름입니다.
		const TCHAR* AssetName;

		// Source PNG의 승인된 가로 픽셀 크기입니다.
		int32 ExpectedWidth;

		// Source PNG의 승인된 세로 픽셀 크기입니다.
		int32 ExpectedHeight;
	};

	// Commandlet 단계별 실패를 구분할 종료 코드입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		PreflightFailed = 61,
		ImportFailed = 62,
		ValidationFailed = 63,
		BindingFailed = 64,
		SaveFailed = 65
	};

	// modular Armor Texture를 생성할 고정 Content 경로입니다.
	const FString VisualDestinationPath = TEXT("/Game/CarFight/UI/HUD/Visual");

	// Production HUD Visual Data의 exact Object Path입니다.
	const TCHAR* HUDVisualDataObjectPath = TEXT("/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default.DA_CFHUDVisual_Default");

	// 기본 2-icon slice에서 허용하는 exact Source/Asset 두 쌍입니다.
	const FArmorTextureImportSpec ArmorIconImportSpecs[] =
	{
		{TEXT("VT04_ArmorIcon_Arrow.png"), TEXT("T_UI_ArmorIcon_Arrow"), 128, 128},
		{TEXT("VT04_ArmorIcon_Chevron2.png"), TEXT("T_UI_ArmorIcon_Chevron2"), 128, 128}
	};

	// -PlateOnly slice에서 허용하는 exact common Plate Source/Asset 계약입니다.
	const FArmorTextureImportSpec ArmorPlateImportSpec =
	{
		TEXT("VT04_ArmorPlate.png"),
		TEXT("T_UI_ArmorPlate"),
		98,
		130
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

	// Source 두 파일, HUDVisualData와 no-overwrite 목적 Asset 상태를 mutation 전에 검증합니다.
	bool ValidatePreflight(const FString& SourceRoot, UCFHUDVisualData*& OutHUDVisualData, FString& OutFailureReason)
	{
		for (const FArmorTextureImportSpec& ImportSpec : ArmorIconImportSpecs)
		{
			// 현재 Source PNG의 절대 경로입니다.
			const FString SourceFilePath = FPaths::Combine(SourceRoot, ImportSpec.SourceFileName);
			if (!IFileManager::Get().FileExists(*SourceFilePath))
			{
				OutFailureReason = FString::Printf(TEXT("Armor icon Source is missing: %s"), *SourceFilePath);
				return false;
			}

			// overwrite를 금지할 목적 Texture의 exact Object Path입니다.
			const FString ExistingTextureObjectPath = MakeTextureObjectPath(ImportSpec.AssetName);
			if (LoadObject<UTexture2D>(nullptr, *ExistingTextureObjectPath))
			{
				OutFailureReason = FString::Printf(TEXT("Refusing to overwrite existing Armor icon Texture: %s"), *ExistingTextureObjectPath);
				return false;
			}
		}

		OutHUDVisualData = LoadObject<UCFHUDVisualData>(nullptr, HUDVisualDataObjectPath);
		if (!OutHUDVisualData)
		{
			OutFailureReason = FString::Printf(TEXT("HUDVisualData load failed: %s"), HUDVisualDataObjectPath);
			return false;
		}

		if (!OutHUDVisualData->ArmorDirectionArrow.IsNull() || !OutHUDVisualData->ArmorDirectionChevron2.IsNull())
		{
			OutFailureReason = TEXT("Refusing to replace non-null ArmorDirectionArrow/ArmorDirectionChevron2 binding");
			return false;
		}
		return true;
	}

	// 하나의 Source PNG를 새 UI Texture로 import하고 no-mip/sRGB UI 설정을 적용합니다.
	UTexture2D* ImportArmorTexture(
		IAssetTools& AssetTools,
		const FString& SourceRoot,
		const FArmorTextureImportSpec& ImportSpec,
		FString& OutFailureReason)
	{
		// 현재 Source PNG의 절대 경로입니다.
		const FString SourceFilePath = FPaths::Combine(SourceRoot, ImportSpec.SourceFileName);

		// 대화상자 없이 신규 Texture를 만들 현재 import task입니다.
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

		// import 후 다시 로드할 exact Texture Object Path입니다.
		const FString TextureObjectPath = MakeTextureObjectPath(ImportSpec.AssetName);

		// import 결과 Texture입니다.
		UTexture2D* ImportedTexture = LoadObject<UTexture2D>(nullptr, *TextureObjectPath);
		if (!ImportedTexture)
		{
			OutFailureReason = FString::Printf(TEXT("Armor Texture import result is missing: %s"), *TextureObjectPath);
			return nullptr;
		}

		ImportedTexture->Modify();
		ImportedTexture->LODGroup = TEXTUREGROUP_UI;
		ImportedTexture->CompressionSettings = TC_EditorIcon;
		ImportedTexture->MipGenSettings = TMGS_NoMipmaps;
		ImportedTexture->SRGB = true;
		ImportedTexture->MarkPackageDirty();
		return ImportedTexture;
	}

	// 하나의 imported Armor Texture가 지정된 Source 크기와 UI Texture 계약을 만족하는지 확인합니다.
	bool ValidateArmorTexture(
		const UTexture2D* Texture,
		const FArmorTextureImportSpec& ImportSpec,
		FString& OutFailureReason)
	{
		if (!Texture)
		{
			OutFailureReason = FString::Printf(TEXT("Armor Texture validation received null Texture: %s"), ImportSpec.AssetName);
			return false;
		}

		// Headless NullRHI import 직후에도 즉시 유효한 원본 Source 가로 크기입니다.
		const int32 SourceSizeX = Texture->Source.GetSizeX();

		// Headless NullRHI import 직후에도 즉시 유효한 원본 Source 세로 크기입니다.
		const int32 SourceSizeY = Texture->Source.GetSizeY();

		if (SourceSizeX != ImportSpec.ExpectedWidth || SourceSizeY != ImportSpec.ExpectedHeight
			|| Texture->LODGroup != TEXTUREGROUP_UI
			|| Texture->CompressionSettings != TC_EditorIcon
			|| Texture->MipGenSettings != TMGS_NoMipmaps
			|| !Texture->SRGB)
		{
			OutFailureReason = FString::Printf(
				TEXT("Armor Texture contract failed: asset=%s expected=%dx%d actual=%dx%d lod=%d compression=%d mip=%d srgb=%s"),
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

	// -PlateOnly가 기존 2-Icon Production 상태를 보존한 채 common Plate만 추가할 수 있는지 검증합니다.
	bool ValidatePlatePreflight(
		const FString& SourceRoot,
		UCFHUDVisualData*& OutHUDVisualData,
		UTexture2D*& OutArrowTexture,
		UTexture2D*& OutChevronTexture,
		FString& OutFailureReason)
	{
		// common Plate Source PNG의 절대 경로입니다.
		const FString PlateSourceFilePath = FPaths::Combine(SourceRoot, ArmorPlateImportSpec.SourceFileName);
		if (!IFileManager::Get().FileExists(*PlateSourceFilePath))
		{
			OutFailureReason = FString::Printf(TEXT("Armor common Plate Source is missing: %s"), *PlateSourceFilePath);
			return false;
		}

		// overwrite를 금지할 common Plate 목적 Texture의 exact Object Path입니다.
		const FString PlateTextureObjectPath = MakeTextureObjectPath(ArmorPlateImportSpec.AssetName);
		if (LoadObject<UTexture2D>(nullptr, *PlateTextureObjectPath))
		{
			OutFailureReason = FString::Printf(TEXT("Refusing to overwrite existing Armor common Plate Texture: %s"), *PlateTextureObjectPath);
			return false;
		}

		OutHUDVisualData = LoadObject<UCFHUDVisualData>(nullptr, HUDVisualDataObjectPath);
		if (!OutHUDVisualData)
		{
			OutFailureReason = FString::Printf(TEXT("HUDVisualData load failed: %s"), HUDVisualDataObjectPath);
			return false;
		}

		if (!OutHUDVisualData->ArmorCommonPlate.IsNull())
		{
			OutFailureReason = TEXT("Refusing to replace non-null ArmorCommonPlate binding");
			return false;
		}

		// 이미 승인·저장된 Arrow Texture의 exact Object Path입니다.
		const FString ArrowTextureObjectPath = MakeTextureObjectPath(ArmorIconImportSpecs[0].AssetName);

		// 이미 승인·저장된 Chevron2 Texture의 exact Object Path입니다.
		const FString ChevronTextureObjectPath = MakeTextureObjectPath(ArmorIconImportSpecs[1].AssetName);

		OutArrowTexture = LoadObject<UTexture2D>(nullptr, *ArrowTextureObjectPath);
		OutChevronTexture = LoadObject<UTexture2D>(nullptr, *ChevronTextureObjectPath);
		if (!OutArrowTexture || !OutChevronTexture)
		{
			OutFailureReason = TEXT("Existing Armor direction Texture validation failed: Arrow or Chevron2 asset is missing");
			return false;
		}

		if (!ValidateArmorTexture(OutArrowTexture, ArmorIconImportSpecs[0], OutFailureReason)
			|| !ValidateArmorTexture(OutChevronTexture, ArmorIconImportSpecs[1], OutFailureReason))
		{
			return false;
		}

		// DataAsset에 저장된 Arrow Soft Reference의 exact Object Path입니다.
		const FString BoundArrowObjectPath = OutHUDVisualData->ArmorDirectionArrow.ToSoftObjectPath().ToString();

		// DataAsset에 저장된 Chevron2 Soft Reference의 exact Object Path입니다.
		const FString BoundChevronObjectPath = OutHUDVisualData->ArmorDirectionChevron2.ToSoftObjectPath().ToString();
		if (!BoundArrowObjectPath.Equals(ArrowTextureObjectPath, ESearchCase::CaseSensitive)
			|| !BoundChevronObjectPath.Equals(ChevronTextureObjectPath, ESearchCase::CaseSensitive))
		{
			OutFailureReason = FString::Printf(
				TEXT("Existing Armor direction binding mismatch: arrow=%s chevron2=%s"),
				*BoundArrowObjectPath,
				*BoundChevronObjectPath);
			return false;
		}
		return true;
	}

	// UObject가 소유한 exact package를 디스크에 저장합니다.
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

		// long package name에서 변환한 .uasset filename입니다.
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

	// -PlateOnly의 common Plate 1개 Import, DataAsset binding과 exact 2-package save를 수행합니다.
	int32 RunPlateOnly(const FString& SourceRoot)
	{
		// PlateOnly preflight에서 로드할 Production HUD Visual Data입니다.
		UCFHUDVisualData* HUDVisualData = nullptr;

		// 기존 승인 상태를 검증할 Arrow Texture입니다.
		UTexture2D* ArrowTexture = nullptr;

		// 기존 승인 상태를 검증할 Chevron2 Texture입니다.
		UTexture2D* ChevronTexture = nullptr;

		// PlateOnly 실패 단계의 구체적인 이유를 보존할 문자열입니다.
		FString FailureReason;
		if (!ValidatePlatePreflight(SourceRoot, HUDVisualData, ArrowTexture, ChevronTexture, FailureReason))
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_PLATE_PREFLIGHT_FAIL %s"), *FailureReason);
			return static_cast<int32>(EExitCode::PreflightFailed);
		}

		// common Plate Source를 import할 Editor-only AssetTools 인터페이스입니다.
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

		// VT04 common Plate Source에서 신규 import된 Texture입니다.
		UTexture2D* PlateTexture = ImportArmorTexture(AssetTools, SourceRoot, ArmorPlateImportSpec, FailureReason);
		if (!PlateTexture)
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_PLATE_IMPORT_FAIL %s"), *FailureReason);
			return static_cast<int32>(EExitCode::ImportFailed);
		}

		if (!ValidateArmorTexture(PlateTexture, ArmorPlateImportSpec, FailureReason))
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_PLATE_VALIDATE_FAIL %s"), *FailureReason);
			return static_cast<int32>(EExitCode::ValidationFailed);
		}

		HUDVisualData->Modify();
		HUDVisualData->ArmorCommonPlate = PlateTexture;
		HUDVisualData->MarkPackageDirty();

		if (!SaveObjectPackage(PlateTexture, FailureReason))
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_PLATE_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(EExitCode::SaveFailed);
		}

		if (!SaveObjectPackage(HUDVisualData, FailureReason))
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_PLATE_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(EExitCode::SaveFailed);
		}

		UE_LOG(
			LogCFUIArmorArt,
			Display,
			TEXT("CF_UI_ARMOR_PLATE_PASS imported=1 bound=1 saved_packages=2 existing_icons_validated=2 plate=%s arrow=%s chevron2=%s sector_mutation=0 silhouette_catalog_mutation=0"),
			*PlateTexture->GetPathName(),
			*ArrowTexture->GetPathName(),
			*ChevronTexture->GetPathName());
		return static_cast<int32>(EExitCode::Success);
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFUIArmorArtCommandlet::UCFUIArmorArtCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// 기본 2-Icon one-shot 또는 -PlateOnly common Plate one-shot을 실행합니다.
int32 UCFUIArmorArtCommandlet::Main(const FString& Params)
{
	// 기존 2-Icon path와 분리된 common Plate 후속 slice 실행 여부입니다.
	const bool bPlateOnly = FParse::Param(*Params, TEXT("PlateOnly"));

	// 이번 commandlet이 읽을 exact VT Source root입니다.
	const FString SourceRoot = CFUIArmorArt::ResolveSourceRoot();
	if (bPlateOnly)
	{
		return CFUIArmorArt::RunPlateOnly(SourceRoot);
	}

	// preflight에서 로드할 Production HUD Visual Data입니다.
	UCFHUDVisualData* HUDVisualData = nullptr;

	// 실패 단계의 구체적인 이유를 보존할 문자열입니다.
	FString FailureReason;
	if (!CFUIArmorArt::ValidatePreflight(SourceRoot, HUDVisualData, FailureReason))
	{
		UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_ART_PREFLIGHT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFUIArmorArt::EExitCode::PreflightFailed);
	}

	// 두 Source PNG를 import할 Editor-only AssetTools 인터페이스입니다.
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

	// import 결과를 AssetName으로 조회할 Texture map입니다.
	TMap<FName, UTexture2D*> ImportedTextures;
	for (const CFUIArmorArt::FArmorTextureImportSpec& ImportSpec : CFUIArmorArt::ArmorIconImportSpecs)
	{
		// 현재 Source에서 새로 import된 Texture입니다.
		UTexture2D* ImportedTexture = CFUIArmorArt::ImportArmorTexture(AssetTools, SourceRoot, ImportSpec, FailureReason);
		if (!ImportedTexture)
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_ART_IMPORT_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFUIArmorArt::EExitCode::ImportFailed);
		}

		if (!CFUIArmorArt::ValidateArmorTexture(ImportedTexture, ImportSpec, FailureReason))
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_ART_VALIDATE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFUIArmorArt::EExitCode::ValidationFailed);
		}
		ImportedTextures.Add(FName(ImportSpec.AssetName), ImportedTexture);
	}

	// imported Arrow Texture입니다.
	UTexture2D* const* ArrowTexturePtr = ImportedTextures.Find(FName(TEXT("T_UI_ArmorIcon_Arrow")));

	// imported Chevron2 Texture입니다.
	UTexture2D* const* ChevronTexturePtr = ImportedTextures.Find(FName(TEXT("T_UI_ArmorIcon_Chevron2")));
	if (!ArrowTexturePtr || !*ArrowTexturePtr || !ChevronTexturePtr || !*ChevronTexturePtr)
	{
		UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_ART_BIND_FAIL imported texture map incomplete"));
		return static_cast<int32>(CFUIArmorArt::EExitCode::BindingFailed);
	}

	HUDVisualData->Modify();
	HUDVisualData->ArmorDirectionArrow = *ArrowTexturePtr;
	HUDVisualData->ArmorDirectionChevron2 = *ChevronTexturePtr;
	HUDVisualData->MarkPackageDirty();

	for (const CFUIArmorArt::FArmorTextureImportSpec& ImportSpec : CFUIArmorArt::ArmorIconImportSpecs)
	{
		// 현재 저장할 imported Texture입니다.
		UTexture2D* const* TexturePtr = ImportedTextures.Find(FName(ImportSpec.AssetName));
		if (!TexturePtr || !CFUIArmorArt::SaveObjectPackage(*TexturePtr, FailureReason))
		{
			UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_ART_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFUIArmorArt::EExitCode::SaveFailed);
		}
	}

	if (!CFUIArmorArt::SaveObjectPackage(HUDVisualData, FailureReason))
	{
		UE_LOG(LogCFUIArmorArt, Error, TEXT("CF_UI_ARMOR_ART_SAVE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFUIArmorArt::EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFUIArmorArt,
		Display,
		TEXT("CF_UI_ARMOR_ART_PASS imported=2 bound=2 saved_packages=3 arrow=%s chevron2=%s sector_mutation=0"),
		*(*ArrowTexturePtr)->GetPathName(),
		*(*ChevronTexturePtr)->GetPathName());
	return static_cast<int32>(CFUIArmorArt::EExitCode::Success);
}
