// Copyright Epic Games, Inc. All Rights Reserved.

// File: CarFight_Re.Build.cs
// Version: v1.2.0
// Changelog:
// - v1.2.0: 싱글플레이 기준선에 맞춰 과거 Visual/Shell 실험 의존성 설명을 정리.
// - v1.1.0: 싱글플레이 전환에 맞춰 CFNetSmooth 모듈 의존성을 제거.
// - v1.0.0: 차량 Visual/Shell 적용 준비용 UCFVehicleSmoothVisComp에서 CFNetSmooth Public API를 참조할 수 있도록 CFNetSmooth 모듈 의존성 추가.
// Migration:
// - CFNetSmooth 플러그인 폴더는 보관하되 CarFight_Re 런타임 모듈은 직접 참조하지 않는다.
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
			"PhysicsCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
