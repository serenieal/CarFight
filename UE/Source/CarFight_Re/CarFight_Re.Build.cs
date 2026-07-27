// Copyright Epic Games, Inc. All Rights Reserved.

// File: CarFight_Re.Build.cs
// Version: v1.6.0
// Changelog:
// - v1.6.0: CF-FQ-024 Combat FX 런타임과 Editor Preview Actor에서 Niagara를 사용하도록 Niagara 의존성을 명시.
// - v1.5.0: TS-P0-06 Editor Automation에서 WidgetBlueprint 생성·컴파일을 검증할 수 있도록 UMGEditor 의존성을 추가.
// - v1.4.0: TS-P0-05 Editor Automation에서 Input Action 에셋 등록을 검증할 수 있도록 AssetRegistry 의존성을 추가.
// - v1.3.0: Editor 자동화 테스트에서 AutomationEditorCommon을 사용할 수 있도록 UnrealEd 의존성을 Editor 빌드에만 추가.
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
			"Niagara",
			"UMG",
			"Slate",
			"SlateCore",
			"ChaosVehicles", 
			"ChaosVehiclesCore", 
			"PhysicsCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// TargetSelect Editor Automation의 테스트 월드 생성과 입력 에셋 등록 검증에만 사용합니다.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
																"UnrealEd",
				"AssetRegistry",
				"UMGEditor"
			});
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
