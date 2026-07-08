// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// CarFight_Re 타깃 설정 버전: v1.1
// Changelog: UE 5.8 소스 엔진 기준 빌드 설정과 include order로 갱신했습니다.
public class CarFight_ReTarget : TargetRules
{
	// CarFight_Re 게임 타깃의 빌드 규칙을 설정합니다.
	public CarFight_ReTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("CarFight_Re");
	}
}
