// Copyright Epic Games, Inc. All Rights Reserved.

// File: CarFight_ReEditor.Build.cs
// Version: v1.8.0
// Changelog:
// - v1.8.0: CF-FQ-040 Guided Builder Step 2 StaticMesh object picker를 위해 Editor-only PropertyEditor 의존성을 추가.
// - v1.7.0: CF-FQ-040 VB-P0-09 AI ResearchDraft USTRUCT JSON handoff를 위해 Editor-only JsonUtilities 의존성을 추가.
// - v1.6.0: CF-FQ-040 CFVRN-1 Unicode NFC canonicalization을 위해 UE 공급 ICU third-party dependency를 추가.
// - v1.5.0: CF-FQ-040 Vehicle Reference Evidence의 Windows UE 5.8 portable SHA-256 계산을 위해 Editor-only OpenSSL third-party dependency를 추가.
// - v1.4.2: CF-FQ-039 Vehicle silhouette Source Candidate를 실제 PNG RGBA로 인코딩하기 위한 Editor-only ImageWrapper 의존성을 추가.
// - v1.4.1: CF-FQ-039 migration Commandlet이 UWidgetTree/UImage 심볼을 직접 사용하므로 Editor 모듈의 direct UMG link 의존성을 추가.
// - v1.4.0: CF-FQ-039 WBP_CFArmorSector one-asset migration Commandlet이 UWidgetBlueprint를 직접 compile/save할 수 있도록 Editor-only UMGEditor 의존성을 추가.
// - v1.3.0: CF-FQ-032 VehiclePanel P2 Source Art의 deterministic Texture import를 위한 Editor-only AssetTools 의존성을 추가.
// - v1.2.0: DAUTH-P0-08J deterministic .cfbatch.json writer를 위해 Editor-only Json 의존성을 추가.
// - v1.1.1: Data Authoring dependency block의 들여쓰기만 교정. 의미 변경 없음.
// - v1.1.0: Data Authoring Profile의 ChaosVehicle enum/class reflection 링크를 위해 ChaosVehicles 의존성을 추가.
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
			"AssetTools",
			"ContentBrowser",
			"ChaosVehicles",
						"InputCore",
			"ImageWrapper",
			"Json",
			"JsonUtilities",
			"LevelEditor",
			"PropertyEditor",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UMG",
			"UMGEditor",
			"UnrealEd"
		});

		// CF-FQ-040 Evidence fingerprint의 SHA-256과 Unicode NFC canonicalization에 UE 공급 OpenSSL/ICU를 사용합니다.
		AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "ICU");
	}
}
