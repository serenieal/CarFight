// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-06-23
// Description: CarFight Editor 전용 도구 모듈 구현입니다.
// Scope: Vehicle DA Wizard 탭 등록, 메뉴 등록, 탭 열기 처리를 담당합니다.
// Changelog:
// - v1.0.0: Window 메뉴에서 열 수 있는 Vehicle DA Wizard 탭을 추가.
// Migration:
// - 기존 에디터 메뉴 항목은 변경하지 않는다.
// - Vehicle DA Wizard는 새 CarFight_ReEditor 모듈의 독립 탭으로 제공한다.

#include "CarFightReEditor.h"

#include "CFVDAWizardTab.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FCarFightReEditorModule"

namespace
{
	// Vehicle DA Wizard 탭 등록에 사용할 고정 탭 식별자입니다.
	static const FName VDAWizardTabName(TEXT("CarFight.VehicleDAWizard"));
}

// Editor 모듈 로드 시 Vehicle DA Wizard 탭과 메뉴를 등록합니다.
void FCarFightReEditorModule::StartupModule()
{
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

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VDAWizardTabName);
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
		TEXT("OpenVehicleDAWizardTab"),
		LOCTEXT("OpenVehicleDAWizardTabLabel", "CarFight Vehicle DA Wizard"),
		LOCTEXT("OpenVehicleDAWizardTabTooltip", "Open the CarFight Vehicle DA validation helper tab."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarFightReEditorModule::OpenVDAWizardTab)));
}

// 등록된 Vehicle DA Wizard 탭을 엽니다.
void FCarFightReEditorModule::OpenVDAWizardTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VDAWizardTabName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarFightReEditorModule, CarFight_ReEditor)
