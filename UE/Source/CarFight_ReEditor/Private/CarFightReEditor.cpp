// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-18
// Description: CarFight Editor 전용 도구 모듈 구현입니다.
// Scope: Vehicle Authoring Workspace와 기존 Vehicle DA Wizard 탭/메뉴 등록을 담당합니다.
// Changelog:
// - v1.2.0: P0-12 UA-01 사용자 피드백에 따라 Vehicle Authoring 탭/메뉴 표시명을 한국어 우선으로 변경. 내부 Tab ID는 유지.
// - v1.1.0: DAUTH-P0-09 Vehicle Authoring Workspace Nomad Tab과 메뉴를 추가하고 기존 Wizard tab/menu를 유지.
// - v1.0.0: Window 메뉴에서 열 수 있는 Vehicle DA Wizard 탭을 추가.
// Migration:
// - 기존 Vehicle DA Wizard 등록/메뉴/spawn 경로는 P0-10 parity 전까지 유지한다.
// - Vehicle Authoring Workspace는 별도 Nomad Tab으로 추가된다.

#include "CarFightReEditor.h"

#include "CFVDAWizardTab.h"
#include "DataAuthoring/CFVehicleAuthoringTab.h"


#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FCarFightReEditorModule"

namespace
{
	// Vehicle Authoring Workspace 탭 등록에 사용할 고정 탭 식별자입니다.
	static const FName VehicleAuthoringTabName(TEXT("CarFight.VehicleAuthoring"));

	// Vehicle DA Wizard 탭 등록에 사용할 기존 고정 탭 식별자입니다.
	static const FName VDAWizardTabName(TEXT("CarFight.VehicleDAWizard"));
}

// Editor 모듈 로드 시 Vehicle Authoring Workspace와 기존 Vehicle DA Wizard 탭/메뉴를 등록합니다.
void FCarFightReEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		VehicleAuthoringTabName,
		FOnSpawnTab::CreateRaw(this, &FCarFightReEditorModule::HandleSpawnAuthoringTab))
		.SetDisplayName(LOCTEXT("VehicleAuthoringTabTitle", "차량 데이터 제작"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		VDAWizardTabName,
		FOnSpawnTab::CreateRaw(this, &FCarFightReEditorModule::HandleSpawnVDAWizardTab))
		.SetDisplayName(LOCTEXT("VDAWizardTabTitle", "Vehicle DA Wizard"))
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

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VehicleAuthoringTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VDAWizardTabName);
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

// Vehicle DA Wizard 탭 인스턴스를 생성합니다.
TSharedRef<SDockTab> FCarFightReEditorModule::HandleSpawnVDAWizardTab(const FSpawnTabArgs& InSpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCFVDAWizardTab)
		];
}

// Level Editor Window 메뉴에 Vehicle DA Wizard 열기 항목을 추가합니다.
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
		TEXT("OpenVehicleAuthoringTab"),
		LOCTEXT("OpenVehicleAuthoringTabLabel", "CarFight 차량 데이터 제작"),
		LOCTEXT("OpenVehicleAuthoringTabTooltip", "CarFight 단일 차량 데이터 제작 작업창을 엽니다."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarFightReEditorModule::OpenAuthoringTab)));

	CarFightSection.AddMenuEntry(
		TEXT("OpenVehicleDAWizardTab"),
		LOCTEXT("OpenVehicleDAWizardTabLabel", "CarFight Vehicle DA Wizard"),
		LOCTEXT("OpenVehicleDAWizardTabTooltip", "Open the CarFight Vehicle DA validation helper tab."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarFightReEditorModule::OpenVDAWizardTab)));
}

// 등록된 Vehicle Authoring Workspace 탭을 엽니다.
void FCarFightReEditorModule::OpenAuthoringTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VehicleAuthoringTabName);
}

// 등록된 Vehicle DA Wizard 탭을 엽니다.
void FCarFightReEditorModule::OpenVDAWizardTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VDAWizardTabName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarFightReEditorModule, CarFight_ReEditor)
