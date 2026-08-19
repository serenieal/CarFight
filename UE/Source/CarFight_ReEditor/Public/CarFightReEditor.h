// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-18
// Description: CarFight Editor 전용 도구 모듈 진입점입니다.
// Scope: Vehicle Authoring Workspace와 기존 Vehicle DA Wizard 탭 등록/메뉴 연결을 담당합니다.
// Changelog:
// - v1.2.0: P0-12 UA-01 한국어 우선 Vehicle Authoring 탭/메뉴 표시와 문서 버전을 동기화.
// - v1.1.0: DAUTH-P0-09 Vehicle Authoring Workspace Nomad Tab/Window 메뉴 진입점을 추가하고 기존 Wizard를 병행 유지.
// - v1.0.0: Vehicle DA Wizard Nomad Tab과 Window 메뉴 진입점을 추가.
// Migration:
// - SCFVDAWizardTab은 P0-12 USER Acceptance와 DG/DEL Gate 완료 전까지 삭제/대체하지 않고 별도 legacy 탭으로 유지한다.
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
		// Vehicle Authoring Workspace 탭 인스턴스를 생성합니다.
	TSharedRef<SDockTab> HandleSpawnAuthoringTab(const FSpawnTabArgs& InSpawnTabArgs);

	// Vehicle DA Wizard 탭 인스턴스를 생성합니다.
	TSharedRef<SDockTab> HandleSpawnVDAWizardTab(const FSpawnTabArgs& InSpawnTabArgs);

	// Level Editor Window 메뉴에 CarFight Authoring/Wizard 열기 항목을 추가합니다.
	void RegisterMenus();

	// 등록된 Vehicle Authoring Workspace 탭을 엽니다.
	void OpenAuthoringTab();

	// 등록된 Vehicle DA Wizard 탭을 엽니다.
	void OpenVDAWizardTab();
};
