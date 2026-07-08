// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// CarFight_ReEditor 타깃 설정 버전: v1.1
// Changelog: UE 5.8 소스 엔진 기준 빌드 설정과 include order로 갱신했습니다.
public class CarFight_ReEditorTarget : TargetRules
{
	// CarFight_ReEditor 에디터 타깃의 빌드 규칙을 설정합니다.
	public CarFight_ReEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("CarFight_Re");
		ExtraModuleNames.Add("CarFight_ReEditor");
	}
}
