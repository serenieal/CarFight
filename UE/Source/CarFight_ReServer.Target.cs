// Copyright Epic Games, Inc. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-05-22
// Description: CarFight_Re Dedicated Server 빌드 타깃입니다.

using UnrealBuildTool;
using System.Collections.Generic;

// CarFight_Re 서버 전용 빌드 타깃을 정의합니다.
public class CarFight_ReServerTarget : TargetRules
{
	// Dedicated Server 빌드 설정을 초기화합니다.
	public CarFight_ReServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("CarFight_Re");
	}
}
