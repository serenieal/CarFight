// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-09-11
// Description: CarFight Editor 전용 도구 모듈 구현입니다.
// Scope: Guided Vehicle Builder, Vehicle Authoring과 Data Asset Manager 탭/메뉴 등록을 담당합니다.
// Changelog:
// - v1.6.0: CF-FQ-038 DEL6 compatibility retirement로 Deprecated Vehicle DA Wizard hidden tab/spawner/open entry를 제거.
// - v1.5.0: CF-FQ-045 DAM-P0-03 CarFight.DataAssetManager Nomad Tab과 Window 메뉴 진입을 추가.
// - v1.4.0: CF-FQ-040 VB-P0-09 준비를 위해 별도 CarFight.VehicleBuilder Guided Shell 탭과 Window 메뉴 진입을 추가.
// - v1.3.0: DAUTH DG1~DG5 PASS에 따라 Legacy Vehicle DA Wizard의 기본 Window 메뉴 진입만 제거. hidden tab spawner와 spawn 함수는 DEL Gate 전까지 유지.
// - v1.2.0: P0-12 UA-01 사용자 피드백에 따라 Vehicle Authoring 탭/메뉴 표시명을 한국어 우선으로 변경. 내부 Tab ID는 유지.
// - v1.1.0: DAUTH-P0-09 Vehicle Authoring Workspace Nomad Tab과 메뉴를 추가하고 기존 Wizard tab/menu를 유지.
// - v1.0.0: Window 메뉴에서 열 수 있는 Vehicle DA Wizard 탭을 추가.
// Migration:
// - v1.6.0부터 Legacy Vehicle DA Wizard tab identity는 더 이상 등록되지 않습니다. 차량 제작은 Guided Builder, 전문 수동 편집/복구는 Vehicle Authoring Workspace를 사용합니다.
// - 사용자 기본 진입은 `CarFight 차량 데이터 제작` Workspace 하나로 통일한다.

#include "CarFightReEditor.h"

#include "DataAuthoring/CFVehicleAuthoringTab.h"
#include "DataAuthoring/CFVehicleBuilderTab.h"
#include "DataManagement/CFDAManagementTab.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FCarFightReEditorModule"

namespace
{
	// Guided Vehicle Builder 탭 등록에 사용할 고정 탭 식별자입니다.
	static const FName VehicleBuilderTabName(TEXT("CarFight.VehicleBuilder"));

	// Vehicle Authoring Workspace 탭 등록에 사용할 고정 탭 식별자입니다.
	static const FName VehicleAuthoringTabName(TEXT("CarFight.VehicleAuthoring"));

	// Data Asset Manager 탭 등록에 사용할 고정 탭 식별자입니다.
	static const FName DataAssetManagerTabName(TEXT("CarFight.DataAssetManager"));

}

// Editor 모듈 로드 시 Guided Builder, Vehicle Authoring Workspace와 Data Asset Manager 탭/메뉴를 등록합니다.
void FCarFightReEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		VehicleBuilderTabName,
		FOnSpawnTab::CreateRaw(this, &FCarFightReEditorModule::HandleSpawnBuilderTab))
		.SetDisplayName(LOCTEXT("VehicleBuilderTabTitle", "차량 제작 가이드"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		VehicleAuthoringTabName,
		FOnSpawnTab::CreateRaw(this, &FCarFightReEditorModule::HandleSpawnAuthoringTab))
		.SetDisplayName(LOCTEXT("VehicleAuthoringTabTitle", "차량 데이터 제작"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		DataAssetManagerTabName,
		FOnSpawnTab::CreateRaw(this, &FCarFightReEditorModule::HandleSpawnDataAssetManagerTab))
		.SetDisplayName(LOCTEXT("DataAssetManagerTabTitle", "CarFight 데이터 관리"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);


	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCarFightReEditorModule::RegisterMenus));
}

// Editor 모듈 언로드 시 등록한 탭과 메뉴 소유권을 정리합니다.
void FCarFightReEditorModule::ShutdownModule()
{
	if (UToolMenus::TryGet())
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VehicleBuilderTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VehicleAuthoringTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DataAssetManagerTabName);
}

// Guided Vehicle Builder 탭 인스턴스를 생성합니다.
TSharedRef<SDockTab> FCarFightReEditorModule::HandleSpawnBuilderTab(const FSpawnTabArgs& InSpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCFVehicleBuilderTab)
		];
}

// Vehicle Authoring Workspace 탭 인스턴스를 생성합니다.
TSharedRef<SDockTab> FCarFightReEditorModule::HandleSpawnAuthoringTab(const FSpawnTabArgs& InSpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCFVehicleAuthoringTab)
		];
}

// Data Asset Manager 탭 인스턴스를 생성합니다.
TSharedRef<SDockTab> FCarFightReEditorModule::HandleSpawnDataAssetManagerTab(const FSpawnTabArgs& InSpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCFDAManagementTab)
		];
}


// Level Editor Window 메뉴에는 Guided Builder와 Current Vehicle Authoring Workspace를 기본 진입으로 등록합니다.
void FCarFightReEditorModule::RegisterMenus()
{
	// 현재 모듈을 ToolMenus 소유자로 등록하기 위한 스코프 객체입니다.
	FToolMenuOwnerScoped OwnerScoped(this);

	// Unreal Level Editor의 Window 메뉴입니다.
	UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
	if (!WindowMenu)
	{
		return;
	}

	// CarFight 에디터 도구 메뉴 섹션입니다.
	FToolMenuSection& CarFightSection = WindowMenu->FindOrAddSection(TEXT("CarFight"));
	CarFightSection.AddMenuEntry(
		TEXT("OpenVehicleBuilderTab"),
		LOCTEXT("OpenVehicleBuilderTabLabel", "CarFight 차량 제작 가이드"),
		LOCTEXT("OpenVehicleBuilderTabTooltip", "실존 차량 Reference부터 Driving까지 한 단계씩 진행하는 Guided Vehicle Builder를 엽니다."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarFightReEditorModule::OpenBuilderTab)));

	CarFightSection.AddMenuEntry(
		TEXT("OpenVehicleAuthoringTab"),
		LOCTEXT("OpenVehicleAuthoringTabLabel", "CarFight 차량 데이터 제작"),
		LOCTEXT("OpenVehicleAuthoringTabTooltip", "CarFight 단일 차량 데이터 제작 작업창을 엽니다."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarFightReEditorModule::OpenAuthoringTab)));

	CarFightSection.AddMenuEntry(
		TEXT("OpenDataAssetManagerTab"),
		LOCTEXT("OpenDataAssetManagerTabLabel", "CarFight 데이터 관리"),
		LOCTEXT("OpenDataAssetManagerTabTooltip", "CarFight DataAsset을 Domain/Type/Asset/Health/Reference 기준으로 탐색하고 검사합니다."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarFightReEditorModule::OpenDataAssetManagerTab)));

}

// 등록된 Guided Vehicle Builder 탭을 엽니다.
void FCarFightReEditorModule::OpenBuilderTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VehicleBuilderTabName);
}

// 등록된 Vehicle Authoring Workspace 탭을 엽니다.
void FCarFightReEditorModule::OpenAuthoringTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VehicleAuthoringTabName);
}

// 등록된 Data Asset Manager 탭을 엽니다.
void FCarFightReEditorModule::OpenDataAssetManagerTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(DataAssetManagerTabName);
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarFightReEditorModule, CarFight_ReEditor)
