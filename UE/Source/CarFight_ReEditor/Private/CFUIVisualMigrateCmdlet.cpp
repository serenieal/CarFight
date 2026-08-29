// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFUIVisualMigrateCmdlet.cpp
// Version: v1.0.1
// Date: 2026-08-25
// Description: CF-FQ-039 WBP_CFArmorSector layout-preserving visual migration 전용 Editor Commandlet 구현입니다.
// Changelog:
// - v1.0.1: existing Image_DirectionIcon도 Bridge GUID repair 후 clean compile하며, migration/metadata repair가 실제 package를 dirty하게 만든 경우에만 exact package를 저장하도록 idempotent 검증 경로를 강화.
// - v1.0.0: exact WBP_CFArmorSector load → existing-tree preflight → ApplyArmorVisualMigrationResult → compile → Image_DirectionIcon postcondition → exact package save 흐름을 추가.
// Migration:
// - 이 Commandlet은 SourceArt를 읽거나 Texture를 Import하지 않습니다.
// - DA_CFHUDVisual_Default, WBP_CFArmorBodyMap, VehiclePanel과 다른 Asset package를 저장하지 않습니다.
// - 이미 Image_DirectionIcon이 UImage로 존재하면 GUID metadata와 clean compile을 검증하고 repair가 없을 때 저장0으로 성공합니다.

#include "CFUIVisualMigrateCmdlet.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "UI/CFUIHUDProdEditorBridge.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFUIVisualMigrate, Log, All);

namespace CFUIVisualMigrate
{
	// Migration 대상 Production ArmorSector Widget Blueprint의 정확한 Object Path입니다.
	const TCHAR* ArmorSectorObjectPath = TEXT("/Game/CarFight/UI/HUD/Elements/WBP_CFArmorSector.WBP_CFArmorSector");

	// Commandlet 단계별 실패를 프로세스 종료 코드로 구분합니다.
	enum class EExitCode : int32
	{
		Success = 0,
		AssetLoadFailed = 51,
		ExistingContractFailed = 52,
		MigrationFailed = 53,
		CompileFailed = 54,
		PostconditionFailed = 55,
		SaveFailed = 56
	};

	// 현재 WidgetTree의 Image_DirectionIcon이 정확한 UImage인지 확인합니다.
	bool HasValidDirectionIcon(const UWidgetBlueprint* ArmorSectorBlueprint)
	{
		if (!ArmorSectorBlueprint || !ArmorSectorBlueprint->WidgetTree)
		{
			return false;
		}

		// 현재 WidgetTree에서 Direction Icon 이름을 가진 Widget입니다.
		UWidget* DirectionIconWidget = ArmorSectorBlueprint->WidgetTree->FindWidget(FName(TEXT("Image_DirectionIcon")));
		return Cast<UImage>(DirectionIconWidget) != nullptr;
	}

	// 정확한 ArmorSector Blueprint package 하나만 디스크에 저장합니다.
	bool SaveArmorSectorBlueprint(UWidgetBlueprint* ArmorSectorBlueprint, FString& OutFailureReason)
	{
		if (!ArmorSectorBlueprint)
		{
			OutFailureReason = TEXT("Cannot save null ArmorSector Widget Blueprint");
			return false;
		}

		// WBP_CFArmorSector가 소유된 정확한 package입니다.
		UPackage* ArmorSectorPackage = ArmorSectorBlueprint->GetOutermost();
		if (!ArmorSectorPackage)
		{
			OutFailureReason = TEXT("ArmorSector Widget Blueprint package is missing");
			return false;
		}

		// Long package name을 실제 .uasset 파일 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			ArmorSectorPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// Public standalone Widget Blueprint package 저장 옵션입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;

		ArmorSectorBlueprint->MarkPackageDirty();
		if (!UPackage::SavePackage(ArmorSectorPackage, ArmorSectorBlueprint, *PackageFilename, SavePackageArgs))
		{
			OutFailureReason = FString::Printf(TEXT("ArmorSector package save failed: %s"), *PackageFilename);
			return false;
		}
		return true;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFUIVisualMigrateCommandlet::UCFUIVisualMigrateCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// WBP_CFArmorSector에 Direction Icon 슬롯만 additive migration하고 검증된 경우 해당 package 하나만 저장합니다.
int32 UCFUIVisualMigrateCommandlet::Main(const FString& Params)
{
	(void)Params;

	// Migration 대상 exact Production ArmorSector Widget Blueprint입니다.
	UWidgetBlueprint* ArmorSectorBlueprint = LoadObject<UWidgetBlueprint>(nullptr, CFUIVisualMigrate::ArmorSectorObjectPath);
	if (!ArmorSectorBlueprint || !ArmorSectorBlueprint->WidgetTree || !ArmorSectorBlueprint->WidgetTree->RootWidget)
	{
		UE_LOG(LogCFUIVisualMigrate, Error, TEXT("CF_UI_VISUAL_MIGRATE_LOAD_FAIL path=%s"), CFUIVisualMigrate::ArmorSectorObjectPath);
		return static_cast<int32>(CFUIVisualMigrate::EExitCode::AssetLoadFailed);
	}

	// Migration 전 동일 이름 Widget이 이미 존재했는지 기록해 신규 추가와 repair 검증을 구분합니다.
	UWidget* ExistingDirectionIconWidget = ArmorSectorBlueprint->WidgetTree->FindWidget(FName(TEXT("Image_DirectionIcon")));
	const bool bAlreadyMigrated = ExistingDirectionIconWidget != nullptr;
	if (ExistingDirectionIconWidget && !Cast<UImage>(ExistingDirectionIconWidget))
	{
		UE_LOG(LogCFUIVisualMigrate, Error, TEXT("CF_UI_VISUAL_MIGRATE_EXISTING_CONTRACT_FAIL name=Image_DirectionIcon type=%s"), *ExistingDirectionIconWidget->GetClass()->GetName());
		return static_cast<int32>(CFUIVisualMigrate::EExitCode::ExistingContractFailed);
	}

	if (!UCFUIHUDProdEditorBridge::ApplyArmorVisualMigrationResult(ArmorSectorBlueprint))
	{
		UE_LOG(LogCFUIVisualMigrate, Error, TEXT("CF_UI_VISUAL_MIGRATE_APPLY_FAIL asset=%s"), CFUIVisualMigrate::ArmorSectorObjectPath);
		return static_cast<int32>(CFUIVisualMigrate::EExitCode::MigrationFailed);
	}

	// Bridge가 신규 Widget 또는 누락 GUID metadata를 실제로 변경해 package save가 필요한지 compile 전에 고정합니다.
	const bool bNeedsPackageSave = ArmorSectorBlueprint->GetOutermost() && ArmorSectorBlueprint->GetOutermost()->IsDirty();

	FKismetEditorUtilities::CompileBlueprint(ArmorSectorBlueprint);
	if (!ArmorSectorBlueprint->GeneratedClass || ArmorSectorBlueprint->Status == BS_Error)
	{
		UE_LOG(LogCFUIVisualMigrate, Error, TEXT("CF_UI_VISUAL_MIGRATE_COMPILE_FAIL asset=%s status=%d"), CFUIVisualMigrate::ArmorSectorObjectPath, static_cast<int32>(ArmorSectorBlueprint->Status));
		return static_cast<int32>(CFUIVisualMigrate::EExitCode::CompileFailed);
	}

	if (!CFUIVisualMigrate::HasValidDirectionIcon(ArmorSectorBlueprint))
	{
		UE_LOG(LogCFUIVisualMigrate, Error, TEXT("CF_UI_VISUAL_MIGRATE_POSTCONDITION_FAIL missing=Image_DirectionIcon asset=%s"), CFUIVisualMigrate::ArmorSectorObjectPath);
		return static_cast<int32>(CFUIVisualMigrate::EExitCode::PostconditionFailed);
	}

	// 실제 migration 또는 GUID metadata repair가 있었을 때만 package를 다시 저장합니다.
	int32 SavedAssetCount = 0;
	if (bNeedsPackageSave)
	{
		// Package save 실패 시 가장 구체적인 이유를 로그에 남길 문자열입니다.
		FString FailureReason;
		if (!CFUIVisualMigrate::SaveArmorSectorBlueprint(ArmorSectorBlueprint, FailureReason))
		{
			UE_LOG(LogCFUIVisualMigrate, Error, TEXT("CF_UI_VISUAL_MIGRATE_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFUIVisualMigrate::EExitCode::SaveFailed);
		}
		SavedAssetCount = 1;
	}

	UE_LOG(LogCFUIVisualMigrate, Display, TEXT("CF_UI_VISUAL_MIGRATE_PASS already_migrated=%s saved_assets=%d asset=%s"), bAlreadyMigrated ? TEXT("true") : TEXT("false"), SavedAssetCount, CFUIVisualMigrate::ArmorSectorObjectPath);
	return static_cast<int32>(CFUIVisualMigrate::EExitCode::Success);
}
