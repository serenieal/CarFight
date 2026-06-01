// Copyright Epic Games, Inc. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-05-29
// Description: CarFight_Re Dedicated Server 빌드 타깃입니다. 서버 런처 배포용으로 Live Coding을 제외합니다.

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
		bWithLiveCoding = false;
		ExtraModuleNames.Add("CarFight_Re");
	}
}
