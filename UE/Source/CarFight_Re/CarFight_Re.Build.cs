// Copyright Epic Games, Inc. All Rights Reserved.

// File: CarFight_Re.Build.cs
// Version: v1.0.0
// Changelog:
// - v1.0.0: 차량 Visual/Shell 적용 준비용 UCFVehicleSmoothVisComp에서 CFNetSmooth Public API를 참조할 수 있도록 CFNetSmooth 모듈 의존성 추가.
// Migration:
// - NetSmoothSync 플러그인은 UE/Plugins/CFNetSmooth 서브모듈로 유지한다. 게임 전용 코드는 CarFight_Re 모듈 안에서만 작성한다.
// Purpose: CarFight 런타임 모듈의 Unreal/플러그인 의존성을 명시한다.

using UnrealBuildTool;

public class CarFight_Re : ModuleRules
{
	public CarFight_Re(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
            "ChaosVehicles", 
			"ChaosVehiclesCore", 
			"PhysicsCore",
			"CFNetSmooth"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
