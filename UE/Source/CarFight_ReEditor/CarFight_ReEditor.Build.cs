// Copyright Epic Games, Inc. All Rights Reserved.

// File: CarFight_ReEditor.Build.cs
// Version: v1.10.4
// Changelog:
// - v1.10.4: CF-FQ-058 exact34 Production candidate Resource Catalog가 UNiagaraSystem class contract를 직접 사용하므로 Editor-only Niagara link 의존성을 추가.
// - v1.10.3: P0-07 Final Technical Review에서 native ZIP/XML compile-probe 의존성이 production dependency list에 남아 있지 않음을 확인하고 OpenXLSX Editor-only 구성을 최종 정리.
// - v1.10.2: fresh read로 확인한 실제 OpenXLSX vendor root(ThirdParty/OpenXLSX/OpenXLSX)에 include root를 정확히 재결속.
// - v1.10.1: vendor directory layout fresh 확인 전 include root를 상위로 당긴 시도(Historical, v1.10.2에서 교정).
// - v1.10.0: CF-FQ-058 CCAS-P0-07 repository-vendored OpenXLSX 0.5.1 / pugixml 1.15 / miniz 3.0.2를 Editor-only로 정적 통합하고 build-time network fetch를 제거.
// - v1.9.0: CF-FQ-058 CCAS-P0-07 native .xlsx Workbook adapter compile probe를 위해 Editor-only FileUtilities/XmlParser 의존성을 일시 추가.
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

		// OpenXLSX는 standard C++ exception을 contract로 사용하므로 Editor module에서만 예외를 활성화합니다.
		bEnableExceptions = true;

		// OpenXLSX static build export macro입니다.
		PrivateDefinitions.Add("OPENXLSX_STATIC_DEFINE=1");

		// Repository-vendored third-party include roots입니다.
		string ThirdPartyRoot = System.IO.Path.Combine(ModuleDirectory, "Private", "ThirdParty");
		string OpenXlsxRoot = System.IO.Path.Combine(ThirdPartyRoot, "OpenXLSX", "OpenXLSX");
		PrivateIncludePaths.Add(OpenXlsxRoot);
		PrivateIncludePaths.Add(System.IO.Path.Combine(OpenXlsxRoot, "headers"));
		PrivateIncludePaths.Add(System.IO.Path.Combine(OpenXlsxRoot, "sources"));
		PrivateIncludePaths.Add(System.IO.Path.Combine(ThirdPartyRoot, "pugixml"));
		PrivateIncludePaths.Add(System.IO.Path.Combine(ThirdPartyRoot, "miniz"));

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
			"Niagara",
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
