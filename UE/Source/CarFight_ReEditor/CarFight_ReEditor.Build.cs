// Copyright Epic Games, Inc. All Rights Reserved.

// File: CarFight_ReEditor.Build.cs
// Version: v1.0.0
// Changelog:
// - v1.0.0: Vehicle DA Wizard 에디터 탭을 위한 Editor 전용 모듈을 추가.
// Migration:
// - 게임 런타임 모듈(CarFight_Re)은 그대로 유지한다.
// - 에디터 전용 UI와 메뉴 의존성은 CarFight_ReEditor 모듈에만 둔다.
// Purpose: CarFight 에디터 도구 모듈의 Unreal Editor/Slate 의존성을 명시한다.

using UnrealBuildTool;

public class CarFight_ReEditor : ModuleRules
{
	public CarFight_ReEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"CarFight_Re"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ApplicationCore",
			"AssetRegistry",
			"ContentBrowser",
			"InputCore",
			"LevelEditor",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd"
		});
	}
}
