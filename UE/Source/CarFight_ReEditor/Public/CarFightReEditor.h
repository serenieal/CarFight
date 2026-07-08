// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-06-23
// Description: CarFight Editor 전용 도구 모듈 진입점입니다.
// Scope: Vehicle DA Wizard 탭 등록과 메뉴 연결을 담당합니다.
// Changelog:
// - v1.0.0: Vehicle DA Wizard Nomad Tab과 Window 메뉴 진입점을 추가.
// Migration:
// - 런타임 모듈의 게임 로직 경로는 변경하지 않는다.
// - 에디터 도구는 CarFight_ReEditor 모듈 로드 시에만 활성화된다.

#pragma once

#include "Modules/ModuleManager.h"

class FSpawnTabArgs;
class SDockTab;

class FCarFightReEditorModule : public IModuleInterface
{
public:
	// Editor 모듈 로드 시 Vehicle DA Wizard 탭과 메뉴를 등록합니다.
	virtual void StartupModule() override;

	// Editor 모듈 언로드 시 등록한 탭과 메뉴 소유권을 정리합니다.
	virtual void ShutdownModule() override;

private:
	// Vehicle DA Wizard 탭 인스턴스를 생성합니다.
	TSharedRef<SDockTab> HandleSpawnVDAWizardTab(const FSpawnTabArgs& InSpawnTabArgs);

	// Level Editor Window 메뉴에 Vehicle DA Wizard 열기 항목을 추가합니다.
	void RegisterMenus();

	// 등록된 Vehicle DA Wizard 탭을 엽니다.
	void OpenVDAWizardTab();
};
