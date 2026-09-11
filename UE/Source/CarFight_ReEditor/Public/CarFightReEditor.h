// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-09-11
// Description: CarFight Editor 전용 도구 모듈 진입점입니다.
// Scope: Guided Vehicle Builder, Vehicle Authoring과 Data Asset Manager 탭/메뉴를 담당합니다.
// Changelog:
// - v1.6.0: CF-FQ-038 DEL6 compatibility retirement로 Deprecated Vehicle DA Wizard hidden tab/spawner/open entry를 제거.
// - v1.5.0: CF-FQ-045 DAM-P0-03 Data Asset Manager Nomad Tab/Window 메뉴 진입점을 추가.
// - v1.4.0: CF-FQ-040 Guided Vehicle Builder Nomad Tab/Window 메뉴 진입점을 추가.
// - v1.3.0: DAUTH Deprecated Gate DG1~DG5 PASS 후 Legacy Wizard의 기본 메뉴 진입을 제거하고 hidden tab identity만 DEL Gate 전까지 보존.
// - v1.2.0: P0-12 UA-01 한국어 우선 Vehicle Authoring 탭/메뉴 표시와 문서 버전을 동기화.
// - v1.1.0: DAUTH-P0-09 Vehicle Authoring Workspace Nomad Tab/Window 메뉴 진입점을 추가하고 기존 Wizard를 병행 유지.
// - v1.0.0: Vehicle DA Wizard Nomad Tab과 Window 메뉴 진입점을 추가.
// Migration:
// - v1.6.0부터 Legacy Vehicle DA Wizard는 제거됐습니다. 정상 제작은 Guided Builder, 전문 수동 편집/복구는 Vehicle Authoring Workspace를 사용합니다.
// - 런타임 모듈의 게임 로직 경로는 변경하지 않는다.
// - 에디터 도구는 CarFight_ReEditor 모듈 로드 시에만 활성화된다.

#pragma once

#include "Modules/ModuleManager.h"

class FSpawnTabArgs;
class SDockTab;

class FCarFightReEditorModule : public IModuleInterface
{
public:
	// Editor 모듈 로드 시 Guided Builder, Current Workspace와 Data Asset Manager 탭/메뉴를 등록합니다.
	virtual void StartupModule() override;

	// Editor 모듈 언로드 시 등록한 탭과 메뉴 소유권을 정리합니다.
	virtual void ShutdownModule() override;

private:
	// Guided Vehicle Builder 탭 인스턴스를 생성합니다.
	TSharedRef<SDockTab> HandleSpawnBuilderTab(const FSpawnTabArgs& InSpawnTabArgs);

	// Vehicle Authoring Workspace 탭 인스턴스를 생성합니다.
	TSharedRef<SDockTab> HandleSpawnAuthoringTab(const FSpawnTabArgs& InSpawnTabArgs);

	// Data Asset Manager 탭 인스턴스를 생성합니다.
	TSharedRef<SDockTab> HandleSpawnDataAssetManagerTab(const FSpawnTabArgs& InSpawnTabArgs);


	// Level Editor Window 메뉴에 Guided Builder, Vehicle Authoring, Data Asset Manager 진입을 추가합니다.
	void RegisterMenus();

	// 등록된 Guided Vehicle Builder 탭을 엽니다.
	void OpenBuilderTab();

	// 등록된 Vehicle Authoring Workspace 탭을 엽니다.
	void OpenAuthoringTab();

	// 등록된 Data Asset Manager 탭을 엽니다.
	void OpenDataAssetManagerTab();

};
