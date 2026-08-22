// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-20
// Description: CF-FQ-032 VehiclePanel P2 Source Art import/build commandlet 구현
// Scope: P2 Texture 9종을 UE 5.8 UI Texture 규격으로 import·검증하고 HUD Visual Data 연결, ArmorSector/SpeedGauge/ArmorBodyMap/VehiclePanel Build·Compile·Validate·Save를 수행합니다.
// Changelog:
// - v1.3.0: T_UI_RPMTrack을 Material 데이터 Texture로 취급해 sRGB=false를 강제하고, 동적 SpeedGauge 재생성 전 기존 SpeedArcMaterial 연결을 fail-closed 요구하여 구형 Track-only 상태로 회귀하지 않도록 보호.
// - v1.2.0: 재사용 WBP_CFArmorSector를 P2 Widget dependency로 추가하고 ArmorBodyMap을 해당 Generated Class 6개 구조로 재생성하도록 future reapply 경로를 동기화.
// - v1.1.0: 모든 P2 Texture에 UI Group + UserInterface2D(RGBA) + NoMipmaps + sRGB를 강제하고 저장 전 exact 설정 검증을 추가.
// - v1.0.0: preflight-first, save-last deterministic P2 HUD Asset pipeline 최초 구현.
// Migration:
// - Runtime Gameplay/Presenter와 Radar/Target/Weapon Visual 슬롯은 수정하지 않습니다.
// - 동일 P2 PNG를 재실행하면 동일 이름 Texture2D를 replace import합니다.
// - Build/Validate 실패 전에는 명시적 package save를 수행하지 않습니다.
// - v1.1.0 설정 검증 실패도 package save 전에 fail-closed합니다.
// - v1.3.0 전체 P2 Commandlet은 M_UI_RPMGauge를 새로 만들지 않습니다. 동적 RPM Material이 먼저 승인·연결된 상태만 보호 재적용하며 없으면 저장 전에 중단합니다.

#include "CFHUDArtP2Cmdlet.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "Engine/Blueprint.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureDefines.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFHUDVisualData.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIHUDProdEditorBridge.h"
#include "UI/CFUIStyleData.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFHUDArtP2, Log, All);

namespace CFHUDArtP2
{
	// [v1.0.0] P2 Source Art 한 장의 파일명과 생성할 Texture2D 이름을 묶습니다.
	struct FArtImportSpec
	{
		// [v1.0.0] SourceArt/UI/HUD/P2 아래 PNG 파일명입니다.
		const TCHAR* SourceFileName;

		// [v1.0.0] /Game/CarFight/UI/HUD/Visual 아래 생성할 Texture2D 자산 이름입니다.
		const TCHAR* AssetName;
	};

	// [v1.0.0] Commandlet 단계별 실패를 명확한 프로세스 종료 코드로 구분합니다.
	enum class EExitCode : int32
	{
		Success = 0,
		PreflightFailed = 31,
		ImportFailed = 32,
		VisualBindingFailed = 33,
		WidgetBuildFailed = 34,
		SaveFailed = 35
	};

	// [v1.0.0] P2 Texture를 생성할 고정 Content 경로입니다.
	const FString VisualDestinationPath = TEXT("/Game/CarFight/UI/HUD/Visual");

	// [v1.0.0] P2 9종의 정확한 Source/Asset 이름 목록입니다.
	const FArtImportSpec ArtImportSpecs[] =
	{
		{TEXT("T_UI_VehPanelFrame.png"), TEXT("T_UI_VehPanelFrame")},
		{TEXT("T_UI_VehSil_LF.png"), TEXT("T_UI_VehSil_LF")},
		{TEXT("T_UI_Armor_Front.png"), TEXT("T_UI_Armor_Front")},
		{TEXT("T_UI_Armor_Right.png"), TEXT("T_UI_Armor_Right")},
		{TEXT("T_UI_Armor_Rear.png"), TEXT("T_UI_Armor_Rear")},
		{TEXT("T_UI_Armor_Left.png"), TEXT("T_UI_Armor_Left")},
		{TEXT("T_UI_Armor_Top.png"), TEXT("T_UI_Armor_Top")},
		{TEXT("T_UI_Armor_Bottom.png"), TEXT("T_UI_Armor_Bottom")},
		{TEXT("T_UI_RPMTrack.png"), TEXT("T_UI_RPMTrack")}
	};

	// [v1.0.0] Production HUD Visual Data의 정확한 Object Path입니다.
	const TCHAR* HUDVisualDataObjectPath = TEXT("/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default.DA_CFHUDVisual_Default");

	// [v1.0.0] Production HUD 1080p Layout Data의 정확한 Object Path입니다.
	const TCHAR* HUDLayoutDataObjectPath = TEXT("/Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16.DA_CFHUDLayout_1080_16");

	// [v1.0.0] Production UI Style Data의 정확한 Object Path입니다.
	const TCHAR* UIStyleDataObjectPath = TEXT("/Game/CarFight/UI/Style/DA_CFUIStyle_Default.DA_CFUIStyle_Default");

	// [v1.0.0] Standard Density Data의 정확한 Object Path입니다.
	const TCHAR* StandardDensityObjectPath = TEXT("/Game/CarFight/UI/Density/DA_CFUIDensity_Standard.DA_CFUIDensity_Standard");

	// [v1.0.0] Compact Density Data의 정확한 Object Path입니다.
	const TCHAR* CompactDensityObjectPath = TEXT("/Game/CarFight/UI/Density/DA_CFUIDensity_Compact.DA_CFUIDensity_Compact");

		// [v1.2.0] Production 재사용 ArmorSector Widget Blueprint의 정확한 Object Path입니다.
	const TCHAR* ArmorSectorObjectPath = TEXT("/Game/CarFight/UI/HUD/Elements/WBP_CFArmorSector.WBP_CFArmorSector");

	// [v1.0.0] Production SpeedGauge Widget Blueprint의 정확한 Object Path입니다.
	const TCHAR* SpeedGaugeObjectPath = TEXT("/Game/CarFight/UI/HUD/Elements/WBP_CFSpeedGauge.WBP_CFSpeedGauge");

	// [v1.0.0] Production ArmorBodyMap Widget Blueprint의 정확한 Object Path입니다.
	const TCHAR* ArmorBodyMapObjectPath = TEXT("/Game/CarFight/UI/HUD/Elements/WBP_CFArmorBodyMap.WBP_CFArmorBodyMap");

	// [v1.0.0] Production VehiclePanel Widget Blueprint의 정확한 Object Path입니다.
	const TCHAR* VehiclePanelObjectPath = TEXT("/Game/CarFight/UI/HUD/Panels/WBP_CFVehiclePanel.WBP_CFVehiclePanel");

	// [v1.0.0] SourceArt/UI/HUD/P2의 저장소 절대 경로를 계산합니다.
	FString ResolveSourceRoot()
	{
		// [v1.0.0] UE Project 폴더에서 저장소 루트 SourceArt/P2로 이동한 절대 경로입니다.
		FString SourceRoot = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("../SourceArt/UI/HUD/P2")));
		FPaths::NormalizeDirectoryName(SourceRoot);
		return SourceRoot;
	}

	// [v1.0.0] 지정 UObject 경로를 요구 타입으로 로드하고 실패 사유를 기록합니다.
	template <typename TObjectType>
	TObjectType* LoadRequiredAsset(const TCHAR* ObjectPath, FString& OutFailureReason)
	{
		// [v1.0.0] 현재 Object Path에서 로드된 요구 타입 자산입니다.
		TObjectType* LoadedAsset = LoadObject<TObjectType>(nullptr, ObjectPath);
		if (!LoadedAsset)
		{
			OutFailureReason = FString::Printf(TEXT("Required asset load failed: %s"), ObjectPath);
		}
		return LoadedAsset;
	}

	// [v1.0.0] P2 9종 Source PNG와 Production 의존 자산이 모두 존재하는지 mutation 전에 검증합니다.
	bool ValidatePreflight(const FString& SourceRoot, FString& OutFailureReason)
	{
		for (const FArtImportSpec& ImportSpec : ArtImportSpecs)
		{
			// [v1.0.0] 현재 P2 PNG의 실제 저장소 절대 경로입니다.
			const FString SourceFilePath = FPaths::Combine(SourceRoot, ImportSpec.SourceFileName);
			if (!IFileManager::Get().FileExists(*SourceFilePath))
			{
				OutFailureReason = FString::Printf(TEXT("P2 source PNG is missing: %s"), *SourceFilePath);
				return false;
			}
		}
		return true;
	}

	// [v1.0.0] 목적 Texture2D의 전체 Object Path를 생성합니다.
	FString MakeTextureObjectPath(const TCHAR* AssetName)
	{
		return FString::Printf(TEXT("%s/%s.%s"), *VisualDestinationPath, AssetName, AssetName);
	}

			// [v1.3.0] P2 Texture가 CarFight HUD의 UE 5.8 UI Texture 규격과 역할별 sRGB 계약을 정확히 만족하는지 확인합니다.
	bool ValidateUITextureSettings(const UTexture2D* Texture, const bool bExpectedSRGB, FString& OutFailureReason)
	{
		if (!Texture)
		{
			OutFailureReason = TEXT("UI Texture settings validation received null Texture");
			return false;
		}

				if (Texture->LODGroup != TEXTUREGROUP_UI
			|| Texture->CompressionSettings != TC_EditorIcon
			|| Texture->MipGenSettings != TMGS_NoMipmaps
			|| Texture->SRGB != bExpectedSRGB)
		{
			OutFailureReason = FString::Printf(
								TEXT("UI Texture settings contract failed: asset=%s lod_group=%d compression=%d mip_gen=%d srgb=%s expected_srgb=%s"),
				*Texture->GetPathName(),
				static_cast<int32>(Texture->LODGroup),
				static_cast<int32>(Texture->CompressionSettings),
								static_cast<int32>(Texture->MipGenSettings),
				Texture->SRGB ? TEXT("true") : TEXT("false"),
				bExpectedSRGB ? TEXT("true") : TEXT("false"));
			return false;
		}
		return true;
	}

	// [v1.1.0] P2 PNG 한 장을 동일 이름 Texture2D로 자동 import/reimport하고 UI Texture 규격을 적용한 결과 Texture를 반환합니다.
	UTexture2D* ImportTexture(
		IAssetTools& AssetTools,
		const FString& SourceRoot,
		const FArtImportSpec& ImportSpec,
		FString& OutFailureReason)
	{
		// [v1.0.0] 현재 PNG의 저장소 절대 경로입니다.
		const FString SourceFilePath = FPaths::Combine(SourceRoot, ImportSpec.SourceFileName);

		// [v1.0.0] 대화상자 없이 동일 목적 자산에 import할 Unreal Asset Import Task입니다.
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
		ImportTask->bReplaceExisting = true;
		ImportTask->bSave = false;

		// [v1.0.0] 단일 import 호출에 전달할 현재 Task 목록입니다.
		TArray<UAssetImportTask*> ImportTasks;
		ImportTasks.Add(ImportTask);
		AssetTools.ImportAssetTasks(ImportTasks);

		// [v1.0.0] import 후 반드시 존재해야 하는 정확한 Texture2D Object Path입니다.
		const FString TextureObjectPath = MakeTextureObjectPath(ImportSpec.AssetName);

		// [v1.0.0] import 결과를 정확한 Object Path에서 다시 읽은 Texture2D입니다.
		UTexture2D* ImportedTexture = LoadObject<UTexture2D>(nullptr, *TextureObjectPath);
		if (!ImportedTexture)
		{
			OutFailureReason = FString::Printf(TEXT("Texture import result is missing: %s"), *TextureObjectPath);
			return nullptr;
		}

						ImportedTexture->Modify();
		ImportedTexture->LODGroup = TEXTUREGROUP_UI;
		ImportedTexture->CompressionSettings = TC_EditorIcon;
		ImportedTexture->MipGenSettings = TMGS_NoMipmaps;
		// [v1.3.0] RPM Track은 R/G/B/A 채널을 Material 데이터로 소비하므로 색공간 변환을 금지합니다.
				const bool bIsRpmDataTexture = FCString::Strcmp(ImportSpec.AssetName, TEXT("T_UI_RPMTrack")) == 0;
		ImportedTexture->SRGB = !bIsRpmDataTexture;

		if (!ValidateUITextureSettings(ImportedTexture, !bIsRpmDataTexture, OutFailureReason))
		{
			return nullptr;
		}

		ImportedTexture->MarkPackageDirty();
		return ImportedTexture;
	}

	// [v1.0.0] P2 9종을 모두 import하고 AssetName→Texture2D Map을 반환합니다.
	bool ImportAllTextures(
		const FString& SourceRoot,
		TMap<FName, UTexture2D*>& OutTextures,
		FString& OutFailureReason)
	{
		// [v1.0.0] Editor-only AssetTools module의 실제 import 인터페이스입니다.
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

		for (const FArtImportSpec& ImportSpec : ArtImportSpecs)
		{
			// [v1.0.0] 현재 P2 Source PNG에서 import된 Texture2D입니다.
			UTexture2D* ImportedTexture = ImportTexture(AssetTools, SourceRoot, ImportSpec, OutFailureReason);
			if (!ImportedTexture)
			{
				return false;
			}
			OutTextures.Add(FName(ImportSpec.AssetName), ImportedTexture);
		}
		return true;
	}

	// [v1.0.0] Import Map에서 요구 Texture를 가져오고 누락이면 실패 사유를 기록합니다.
	UTexture2D* FindRequiredTexture(
		const TMap<FName, UTexture2D*>& Textures,
		const TCHAR* AssetName,
		FString& OutFailureReason)
	{
		// [v1.0.0] 요구 AssetName으로 찾은 Texture 포인터입니다.
		UTexture2D* const* FoundTexture = Textures.Find(FName(AssetName));
		if (!FoundTexture || !*FoundTexture)
		{
			OutFailureReason = FString::Printf(TEXT("Imported texture map is missing: %s"), AssetName);
			return nullptr;
		}
		return *FoundTexture;
	}

	// [v1.0.0] P2 9종 중 VehiclePanel이 소유하는 모든 Visual 슬롯을 HUD Visual Data에 연결합니다.
	bool BindVehicleVisualData(
		UCFHUDVisualData* HUDVisualData,
		const TMap<FName, UTexture2D*>& Textures,
		FString& OutFailureReason)
	{
		if (!HUDVisualData)
		{
			OutFailureReason = TEXT("HUDVisualData is null");
			return false;
		}

		// [v1.0.0] VehiclePanel 전체 9-Slice Frame Texture입니다.
		UTexture2D* VehiclePanelFrame = FindRequiredTexture(Textures, TEXT("T_UI_VehPanelFrame"), OutFailureReason);

		// [v1.0.0] Armor Body Map 중앙의 좌향 Vehicle Silhouette Texture입니다.
		UTexture2D* VehicleSilhouette = FindRequiredTexture(Textures, TEXT("T_UI_VehSil_LF"), OutFailureReason);

		// [v1.0.0] Front Armor 왼쪽 방향 Badge Texture입니다.
		UTexture2D* ArmorFront = FindRequiredTexture(Textures, TEXT("T_UI_Armor_Front"), OutFailureReason);

		// [v1.0.0] Right Armor 위쪽 방향 Badge Texture입니다.
		UTexture2D* ArmorRight = FindRequiredTexture(Textures, TEXT("T_UI_Armor_Right"), OutFailureReason);

		// [v1.0.0] Rear Armor 오른쪽 방향 Badge Texture입니다.
		UTexture2D* ArmorRear = FindRequiredTexture(Textures, TEXT("T_UI_Armor_Rear"), OutFailureReason);

		// [v1.0.0] Left Armor 아래쪽 방향 Badge Texture입니다.
		UTexture2D* ArmorLeft = FindRequiredTexture(Textures, TEXT("T_UI_Armor_Left"), OutFailureReason);

		// [v1.0.0] Top Armor 이중 상향 Chevron Badge Texture입니다.
		UTexture2D* ArmorTop = FindRequiredTexture(Textures, TEXT("T_UI_Armor_Top"), OutFailureReason);

		// [v1.0.0] Bottom Armor 이중 하향 Chevron Badge Texture입니다.
		UTexture2D* ArmorBottom = FindRequiredTexture(Textures, TEXT("T_UI_Armor_Bottom"), OutFailureReason);

		// [v1.0.0] 좌측 세로→곡선→상단 수평 RPM Track Texture입니다.
		UTexture2D* RPMTrack = FindRequiredTexture(Textures, TEXT("T_UI_RPMTrack"), OutFailureReason);

		if (!VehiclePanelFrame || !VehicleSilhouette || !ArmorFront || !ArmorRight || !ArmorRear
			|| !ArmorLeft || !ArmorTop || !ArmorBottom || !RPMTrack)
		{
			return false;
		}

		HUDVisualData->Modify();
		HUDVisualData->VehiclePanelFrame = VehiclePanelFrame;
		HUDVisualData->VehicleSilhouette = VehicleSilhouette;
		HUDVisualData->ArmorPlates.FrontPlate = ArmorFront;
		HUDVisualData->ArmorPlates.RightPlate = ArmorRight;
		HUDVisualData->ArmorPlates.RearPlate = ArmorRear;
		HUDVisualData->ArmorPlates.LeftPlate = ArmorLeft;
		HUDVisualData->ArmorPlates.TopPlate = ArmorTop;
		HUDVisualData->ArmorPlates.BottomPlate = ArmorBottom;
				HUDVisualData->SpeedArcTrack = RPMTrack;
		HUDVisualData->MarkPackageDirty();

		// [v1.3.0] 새 SpeedGauge는 Texture를 직접 표시하지 않고 UI Material을 요구하므로 구형 Track-only 상태를 저장 전에 차단합니다.
		if (HUDVisualData->SpeedArcMaterial.IsNull())
		{
			OutFailureReason = TEXT("HUDVisualData SpeedArcMaterial is required for dynamic RPM Gauge before P2 reapply");
			return false;
		}

		if (!HUDVisualData->HasCompleteVehicleArt())
		{
			OutFailureReason = TEXT("HUDVisualData Vehicle Art completeness contract failed after P2 binding");
			return false;
		}
		return true;
	}

	// [v1.0.0] Widget Blueprint를 컴파일하고 GeneratedClass와 Compile 상태를 확인합니다.
	bool CompileWidgetBlueprint(UObject* WidgetBlueprintObject, FString& OutFailureReason)
	{
		// [v1.0.0] Production Widget UObject를 일반 UBlueprint로 해석한 포인터입니다.
		UBlueprint* WidgetBlueprint = Cast<UBlueprint>(WidgetBlueprintObject);
		if (!WidgetBlueprint)
		{
			OutFailureReason = TEXT("Production Widget object is not a UBlueprint");
			return false;
		}

		FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
		if (!WidgetBlueprint->GeneratedClass || WidgetBlueprint->Status == BS_Error)
		{
			OutFailureReason = FString::Printf(TEXT("Blueprint compile failed: %s"), *WidgetBlueprint->GetPathName());
			return false;
		}
		return true;
	}

	// [v1.0.0] 한 Production Widget 역할을 Bridge로 재생성하고 Compile/Validate까지 완료합니다.
		bool BuildCompileValidateWidget(
		UObject* WidgetBlueprintObject,
		const FName WidgetRole,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UCFHUDVisualData* HUDVisualData,
		UObject* SpeedGaugeBlueprintObject,
		UObject* ArmorBodyMapBlueprintObject,
		UObject* ArmorSectorBlueprintObject,
		FString& OutFailureReason)
	{
		if (!UCFUIHUDProdEditorBridge::BuildProductionWidgetResult(
			WidgetBlueprintObject,
			WidgetRole,
			LayoutData,
			StyleData,
			StandardDensityData,
			CompactDensityData,
						HUDVisualData,
			SpeedGaugeBlueprintObject,
			ArmorBodyMapBlueprintObject,
			ArmorSectorBlueprintObject))
		{
			OutFailureReason = FString::Printf(TEXT("Production Widget build failed: %s"), *WidgetRole.ToString());
			return false;
		}

		if (!CompileWidgetBlueprint(WidgetBlueprintObject, OutFailureReason))
		{
			return false;
		}

		if (!UCFUIHUDProdEditorBridge::ValidateProductionWidgetResult(
			WidgetBlueprintObject,
			WidgetRole,
			LayoutData,
			StyleData,
			StandardDensityData,
			CompactDensityData,
						HUDVisualData,
			SpeedGaugeBlueprintObject,
			ArmorBodyMapBlueprintObject,
			ArmorSectorBlueprintObject))
		{
			OutFailureReason = FString::Printf(TEXT("Production Widget validation failed: %s"), *WidgetRole.ToString());
			return false;
		}
		return true;
	}

	// [v1.0.0] 지정 Asset의 package를 정확한 .uasset 파일에 저장합니다.
	bool SaveAssetPackage(UObject* AssetObject, FString& OutFailureReason)
	{
		if (!AssetObject)
		{
			OutFailureReason = TEXT("Cannot save null AssetObject");
			return false;
		}

		// [v1.0.0] 저장할 자산을 소유하는 Unreal package입니다.
		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			OutFailureReason = FString::Printf(TEXT("Asset package is missing: %s"), *AssetObject->GetPathName());
			return false;
		}

		// [v1.0.0] Long package name을 실제 Content .uasset 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// [v1.0.0] Public standalone Editor Asset 저장에 사용할 SavePackage 옵션입니다.
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
}

// [v1.0.0] Headless Editor commandlet 기본 실행 속성을 준비합니다.
UCFHUDArtP2Commandlet::UCFHUDArtP2Commandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// [v1.0.0] P2 Source Art import → DataAsset 연결 → Production Widget 재생성 → 저장 전체 파이프라인을 실행합니다.
int32 UCFHUDArtP2Commandlet::Main(const FString& Params)
{
	(void)Params;

	// [v1.0.0] 모든 단계에서 사용자에게 반환할 가장 구체적인 실패 사유입니다.
	FString FailureReason;

	// [v1.0.0] 저장소 SourceArt/UI/HUD/P2의 절대 경로입니다.
	const FString SourceRoot = CFHUDArtP2::ResolveSourceRoot();
	if (!CFHUDArtP2::ValidatePreflight(SourceRoot, FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_PRECHECK_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::PreflightFailed);
	}

	// [v1.0.0] P2 Texture Reference를 연결할 Production HUD Visual Data입니다.
	UCFHUDVisualData* HUDVisualData = CFHUDArtP2::LoadRequiredAsset<UCFHUDVisualData>(CFHUDArtP2::HUDVisualDataObjectPath, FailureReason);

	// [v1.0.0] VehiclePanel Production Layout을 제공하는 1080p Layout Data입니다.
	UCFHUDLayoutData* LayoutData = CFHUDArtP2::LoadRequiredAsset<UCFHUDLayoutData>(CFHUDArtP2::HUDLayoutDataObjectPath, FailureReason);

	// [v1.0.0] VehiclePanel Production 색상/Font/Icon Style Data입니다.
	UCFUIStyleData* StyleData = CFHUDArtP2::LoadRequiredAsset<UCFUIStyleData>(CFHUDArtP2::UIStyleDataObjectPath, FailureReason);

	// [v1.0.0] Production Standard Density Data입니다.
	UCFUIDensityData* StandardDensityData = CFHUDArtP2::LoadRequiredAsset<UCFUIDensityData>(CFHUDArtP2::StandardDensityObjectPath, FailureReason);

	// [v1.0.0] Production Compact Density Data입니다.
	UCFUIDensityData* CompactDensityData = CFHUDArtP2::LoadRequiredAsset<UCFUIDensityData>(CFHUDArtP2::CompactDensityObjectPath, FailureReason);

		// [v1.2.0] 방향 Label + Plate Image + 실제 Armor Progress를 묶는 Production 재사용 ArmorSector Blueprint입니다.
	UObject* ArmorSectorBlueprint = CFHUDArtP2::LoadRequiredAsset<UObject>(CFHUDArtP2::ArmorSectorObjectPath, FailureReason);

	// [v1.0.0] P2 RPM Track을 소비할 Production SpeedGauge Blueprint입니다.
	UObject* SpeedGaugeBlueprint = CFHUDArtP2::LoadRequiredAsset<UObject>(CFHUDArtP2::SpeedGaugeObjectPath, FailureReason);

	// [v1.0.0] P2 Vehicle/Armor Badge를 소비할 Production ArmorBodyMap Blueprint입니다.
	UObject* ArmorBodyMapBlueprint = CFHUDArtP2::LoadRequiredAsset<UObject>(CFHUDArtP2::ArmorBodyMapObjectPath, FailureReason);

	// [v1.0.0] P2 9-Slice Frame과 두 Element를 조립할 Production VehiclePanel Blueprint입니다.
	UObject* VehiclePanelBlueprint = CFHUDArtP2::LoadRequiredAsset<UObject>(CFHUDArtP2::VehiclePanelObjectPath, FailureReason);

		if (!HUDVisualData || !LayoutData || !StyleData || !StandardDensityData || !CompactDensityData
		|| !ArmorSectorBlueprint || !SpeedGaugeBlueprint || !ArmorBodyMapBlueprint || !VehiclePanelBlueprint)
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_PRECHECK_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::PreflightFailed);
	}

	// [v1.0.0] AssetName을 키로 보관하는 이번 실행의 9개 imported Texture Map입니다.
	TMap<FName, UTexture2D*> ImportedTextures;
	if (!CFHUDArtP2::ImportAllTextures(SourceRoot, ImportedTextures, FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_IMPORT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::ImportFailed);
	}

	if (!CFHUDArtP2::BindVehicleVisualData(HUDVisualData, ImportedTextures, FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_BIND_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::VisualBindingFailed);
	}

		if (!CFHUDArtP2::BuildCompileValidateWidget(
		ArmorSectorBlueprint,
		FName(TEXT("ArmorSector")),
		LayoutData,
		StyleData,
		StandardDensityData,
		CompactDensityData,
		HUDVisualData,
		nullptr,
		nullptr,
		nullptr,
		FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_WIDGET_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::WidgetBuildFailed);
	}

	if (!CFHUDArtP2::BuildCompileValidateWidget(
		SpeedGaugeBlueprint,
		FName(TEXT("SpeedGauge")),
		LayoutData,
		StyleData,
		StandardDensityData,
		CompactDensityData,
				HUDVisualData,
		nullptr,
		nullptr,
		nullptr,
		FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_WIDGET_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::WidgetBuildFailed);
	}

	if (!CFHUDArtP2::BuildCompileValidateWidget(
		ArmorBodyMapBlueprint,
		FName(TEXT("ArmorBodyMap")),
		LayoutData,
		StyleData,
		StandardDensityData,
		CompactDensityData,
				HUDVisualData,
		nullptr,
		nullptr,
		ArmorSectorBlueprint,
		FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_WIDGET_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::WidgetBuildFailed);
	}

	if (!CFHUDArtP2::BuildCompileValidateWidget(
		VehiclePanelBlueprint,
		FName(TEXT("VehiclePanel")),
		LayoutData,
		StyleData,
		StandardDensityData,
		CompactDensityData,
				HUDVisualData,
		SpeedGaugeBlueprint,
		ArmorBodyMapBlueprint,
		ArmorSectorBlueprint,
		FailureReason))
	{
		UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_WIDGET_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFHUDArtP2::EExitCode::WidgetBuildFailed);
	}

	// [v1.0.0] Save 단계에서 중복 없이 저장할 정확한 Texture/DA/Widget Asset 목록입니다.
	TArray<UObject*> AssetsToSave;
	for (const CFHUDArtP2::FArtImportSpec& ImportSpec : CFHUDArtP2::ArtImportSpecs)
	{
		// [v1.0.0] 현재 AssetName에 대응하는 imported Texture입니다.
		UTexture2D* const* ImportedTexture = ImportedTextures.Find(FName(ImportSpec.AssetName));
		if (!ImportedTexture || !*ImportedTexture)
		{
			FailureReason = FString::Printf(TEXT("Save list texture is missing: %s"), ImportSpec.AssetName);
			UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFHUDArtP2::EExitCode::SaveFailed);
		}
		AssetsToSave.Add(*ImportedTexture);
	}
		AssetsToSave.Add(HUDVisualData);
	AssetsToSave.Add(ArmorSectorBlueprint);
	AssetsToSave.Add(SpeedGaugeBlueprint);
	AssetsToSave.Add(ArmorBodyMapBlueprint);
	AssetsToSave.Add(VehiclePanelBlueprint);

	for (UObject* AssetToSave : AssetsToSave)
	{
		if (!CFHUDArtP2::SaveAssetPackage(AssetToSave, FailureReason))
		{
			UE_LOG(LogCFHUDArtP2, Error, TEXT("CFHUD_P2_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFHUDArtP2::EExitCode::SaveFailed);
		}
	}

		UE_LOG(LogCFHUDArtP2, Display, TEXT("CFHUD_P2_PASS ImportedTextures=9 BoundVehicleSlots=9 Widgets=4 SavedAssets=%d"), AssetsToSave.Num());
	return static_cast<int32>(CFHUDArtP2::EExitCode::Success);
}
