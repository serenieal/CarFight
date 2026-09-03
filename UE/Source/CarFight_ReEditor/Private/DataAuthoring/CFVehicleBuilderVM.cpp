// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderVM.cpp
// Version: v1.27.0
// Date: 2026-09-02
// Description: Guided Vehicle Builder Shell ViewModel + CF-FQ-042 Vehicle ID naming/create 구현입니다.
// Changelog:
// - v1.27.0: P0-07 UAT에서 Step 8 USER Driving PASS를 Target path/hash 기반 persistent Recipe receipt로 승격. same DefinitionHash에서는 benchmark RunId 변경/Editor 재기동에도 PASS 유지, Target drift에서만 stale. Recipe receipt dirty는 Step 8 saved Target gate를 막지 않음.
// - v1.26.0: P0-07 UAT에서 완료 Wagon이 local Reference token 유실 후 Step 1 Ready → Step 5~8 Locked로 되감기는 회귀를 교정. exact persistent BuilderCommitReceipt EvidenceId/Fingerprint/path를 durable USER acceptance provenance로 재사용하고 Evidence drift는 Stale로 유지.
// - v1.25.1: VMG-P0-04 코드감사에서 Step 6 전체를 read-only라고 설명하던 stale USER 문구를 교정. 8영역 Gameplay Guidance/Socket 진단만 R0 read-only이고 Standard Mount 패널의 explicit Recipe write는 허용됨을 정확히 표시.
// - v1.25.0: CF-FQ-043 VMG-P0-04 Standard 1:1 Mount complete-draft commit을 추가. 새 Mount_<Hardpoint> ID는 Recipe/Target collision을 fail-closed하고 existing ID/bExposedModule은 stable 보존하며 EquipmentPreset CanUseOnMount를 commit 전 검증.
// - v1.24.1: VMG-P0-03 Hardpoint Socket draft는 Resolver/R3의 fail-closed blocker를 유지한 채 Builder에서만 expected Step 3 Ready diagnostic read로 허용하도록 narrow gate를 추가.
// - v1.24.0: CF-FQ-043 VMG-P0-03 Standard Hardpoint category→stable ID/socket creation, Recipe+Target collision-aware max+1 numbering, Mode-authoritative Step 3 evaluator와 conditional non-Hardpoint Socket projection을 추가.
// - v1.23.0: CF-FQ-043 Guided creation에 explicit Hardpoint Plan opt-in을 binding하고 Recipe-owned Mode transaction/readback, prepared workflow invalidation, Mount/Hardpoint typed remove Builder wrapper를 추가. prepared approval만 폐기하며 완료된 DefinitionApply guarded Undo token은 보존.
// - v1.22.0: 기존 차량 선택 상태에서 + 새 차량 만들기 진입 시 Authoring selection을 함께 해제해 Browser refresh가 이전 차량을 자동 복원하며 New Vehicle mode를 해제하는 회귀를 차단.
// - v1.21.0: Vehicle ID ASCII alnum/_ validation과 deterministic DA_Vehicle_/DA_Recipe_ default identity builder를 추가해 Explicit New Vehicle과 Mesh Candidate Quick Start가 같은 naming owner를 사용하도록 연결.
// - v1.20.0: Blank/Arbitrary/Reused/Mesh-only creation을 공통 Guided request로 통합하고, 생성 성공 뒤 exact Browser row를 fresh read해 Builder current target adoption을 검증.
// - v1.19.0: 생성자에서 Stable 8-Step을 즉시 초기화하고, selection-independent 신규 차량 진입 state와 no-selection Step 1 guidance를 추가.
// - v1.18.0: PhysicsDraft schema v3 EngineCurveReview를 Step 5 request/receipt resume에 연결하고 schema v2 legacy draft read compatibility를 유지.
// - v1.17.0: Existing Reference Evidence complete replacement R1의 Step 1 prepared Preview→AuthoringWrite Commit flow를 추가하고, existing Evidence와 다른 ResearchDraft를 refresh candidate로 load 가능하게 분리.
// - v1.16.0: Final Review Transmission diagnostic에 기어별 RPM retention을 표시해 generic fixed-shift spacing 검토 정보를 완성.
// - v1.15.0: Guided Mesh 신규 record를 VehicleSpecificRequired로 생성하고 PhysicsDraft v2 TransmissionReview를 Step 5 preview/receipt resume에 연결. Final Review summary에 Transmission diagnostic을 노출.
// - v1.14.0: Guided Builder refresh에서 Resolver가 Blocked 상태로 false를 반환하면서 OutError가 비어 USER에게 '현재 상태 확인 실패:'만 보이던 문제를 교정. Blocked/Error ResolverValidation issue를 stable IssueCode + 메시지로 surface.
// - v1.13.0: Step 4 Ready/NotCaptured가 Step 7 deferred Layout Apply를 기다리는 정상 상태일 때만 navigation/Step 5 prerequisite로 인정해 actual Wagon E2E deadlock을 제거. Blocked/Stale은 그대로 차단.
// - v1.12.0: actual Wagon E2E에서 Recipe-only 사전 authoring revision이 NewVehicle lifecycle을 소모하지 않게 교정하고, private Profile bootstrap 전의 expected Resolver Blocked read를 Builder selection/refresh에서 안전하게 유지.
// - v1.11.0: SocketScaleFromChassis의 Step2 canonical Wheel, Step3 Socket Scale/axle size, Step4 RelativeScale stale 검증 추가.
// - v1.10.0: E2E에서 발견된 Step 2 Wheel Mesh 지정 UX 공백을 existing typed AssetIntent Recipe-only commit으로 연결하고 commit 뒤 Builder state fresh 재평가를 추가.
// - v1.9.0: USER 피드백 기반 Step 3 소켓 준비 UX를 위해 effective Wheel Socket/optional Gameplay Socket/current Chassis/found-state read-only projection을 추가하고 Step 제목을 명확화.
// - v1.8.0: VB-P0-09 Step 8에서 existing VB-P0-08 saved VehicleData benchmark result, active PIE transient USER test-drive, exact benchmark-bound USER Driving acceptance를 Guided Shell VM에 연결.
// - v1.7.0: VB-P0-09 Step 7에서 existing ReadBuilderFinalReview R0 → explicit DefinitionApply → exact Builder-owned guarded Undo를 Guided Shell VM에 연결.
// - v1.6.0: VB-P0-09 Step 6에서 existing ReadBuilderGameplayGuidance R0 authority를 8영역 completeness/USER Socket guidance/current pending diff projection으로 Guided Shell에 연결.
// - v1.5.0: VB-P0-09 Step 5 PhysicsDraft JSON, accepted Evidence/private 4 Profile typed preview→explicit commit, persistent receipt 기반 Complete/Stale evaluator를 연결.
// - v1.4.0: VB-P0-09 Step 1 ResearchDraft JSON, exact Evidence discovery, baseline-preserving Companion approval/commit, local Reference review token과 resume을 연결.
// - v1.3.1: Step Definition 순서가 evaluator dispatch 순서까지 소유하도록 StepId dispatcher를 추가해 reorder 수정 지점을 단일화.
// - v1.3.0: Step Definition 단일 owner, StepId 기반 lookup/setter, Step별 evaluator로 분리해 Step 추가·제거·순서 변경 영향 범위를 축소.
// - v1.2.0: VB-P0-03 고정 계약대로 fresh FCFVehicleAssetSnapshot 기반 MeshPrep/SocketGuide/LayoutCapture read-only evaluator를 연결.
// - v1.1.0: Mesh-only 후보의 safe VehicleData+Recipe Preview→explicit commit을 기존 Authoring VM에 그대로 위임.
// - v1.0.0: 기존 Authoring VM selection을 재사용하고, 아직 provider가 연결되지 않은 Step을 정직하게 Locked/Ready로 표시.
// Migration:
// - v1.22.0부터 Explicit New Vehicle 진입은 이전 Authoring selection/cache를 selection level에서 해제합니다. Browser cache 자체는 유지하며 Asset 생성/Save/Apply는 수행하지 않습니다.
// - v1.21.0부터 Guided 신규 차량의 일반 naming은 Vehicle ID 한 칸을 기본 owner로 사용합니다. invalid 입력은 sanitize하지 않고 fail-closed하며, package/name collision·최종 경로·type 판정은 기존 PreviewVehicleRecords/ValidateNewAssetIdentity authority를 유지합니다.
// - v1.20.0부터 explicit New Vehicle과 Mesh-only Quick Start는 동일 Guided creation helper를 사용하며 VehicleSpecificRequired를 강제합니다. 생성 Chassis는 Recipe AssetIntent에만 기록하고 VehicleData Apply/Save는 0입니다. 생성 성공과 post-create Builder adoption 실패는 분리합니다.
// - v1.19.0 신규 차량 진입은 transient Builder state만 변경하며 CreateVehicleRecords/VehicleData Apply/Save를 호출하지 않습니다. 실제 Blank/ChassisMesh 생성은 후속 VBCUX Gate가 연결합니다.
// - Recipe-only AuthoringRevision은 NewVehicle→CompleteExisting lifecycle 전환 근거로 사용하지 않습니다. Import/Apply/Builder Profile commit, private Profile, Evidence가 실제 lifecycle 경계를 소유합니다.
// - Step Complete를 임의 bool로 저장하지 않습니다. 새로고침할 때 current truth에서 다시 파생합니다.

#include "DataAuthoring/CFVehicleBuilderVM.h"

#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFWheelSizeUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "JsonObjectConverter.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	// Step state를 USER-facing 한국어로 설명합니다.
	const TCHAR* StepStateText(const ECFVehicleBuilderStepState State)
	{
		switch (State)
		{
		case ECFVehicleBuilderStepState::Unavailable: return TEXT("사용 불가");
		case ECFVehicleBuilderStepState::Locked: return TEXT("잠김");
		case ECFVehicleBuilderStepState::Ready: return TEXT("진행 가능");
		case ECFVehicleBuilderStepState::Complete: return TEXT("완료");
		case ECFVehicleBuilderStepState::Blocked: return TEXT("막힘");
		case ECFVehicleBuilderStepState::Stale: return TEXT("다시 확인 필요");
		default: return TEXT("알 수 없음");
		}
	}

	// Resolver operation 자체는 성공했지만 ResolveStatus가 Blocked일 때 USER-facing 실패 이유를 구조화 issue에서 복원합니다.
	FString BuildResolverFailureMessage(const FCFVehicleResolveReadResult& ResolveRead)
	{
		TArray<FString> BlockingIssueLines;
		for (const FCFVehicleValidationIssue& Issue : ResolveRead.ResolveResult.ResolverValidation)
		{
			if (Issue.Severity != ECFVehicleValidationSeverity::Blocked
				&& Issue.Severity != ECFVehicleValidationSeverity::Error)
			{
				continue;
			}

			BlockingIssueLines.Add(FString::Printf(
				TEXT("[%s] %s"),
				*Issue.IssueCode.ToString(),
				Issue.Message.IsEmpty() ? TEXT("상세 메시지 없음") : *Issue.Message));
		}

		if (!BlockingIssueLines.IsEmpty())
		{
			return FString::Join(BlockingIssueLines, LINE_TERMINATOR);
		}

		return ResolveRead.Operation.Message;
	}

	// Gameplay Guidance 영역을 USER-facing 한국어 이름으로 변환합니다.
	const TCHAR* GameplayAreaText(const ECFBuilderGameplayArea Area)
	{
		switch (Area)
		{
		case ECFBuilderGameplayArea::Durability: return TEXT("내구도");
		case ECFBuilderGameplayArea::Defense: return TEXT("방어");
		case ECFBuilderGameplayArea::DestroyedFx: return TEXT("파괴 FX");
		case ECFBuilderGameplayArea::Hardpoints: return TEXT("하드포인트");
		case ECFBuilderGameplayArea::MountProfiles: return TEXT("마운트");
		case ECFBuilderGameplayArea::DriveState: return TEXT("주행 상태");
		case ECFBuilderGameplayArea::WheelVisual: return TEXT("휠 비주얼");
		case ECFBuilderGameplayArea::FittingMass: return TEXT("피팅/질량");
		default: return TEXT("알 수 없음");
		}
	}

	// Gameplay Guidance 상태를 USER-facing 한국어로 변환합니다.
	const TCHAR* GameplayGuidanceStateText(const ECFBuilderGuidanceState State)
	{
		switch (State)
		{
		case ECFBuilderGuidanceState::Complete: return TEXT("완료");
		case ECFBuilderGuidanceState::Optional: return TEXT("선택 사항");
		case ECFBuilderGuidanceState::NeedsReview: return TEXT("확인 필요");
		case ECFBuilderGuidanceState::Blocked: return TEXT("막힘");
		default: return TEXT("알 수 없음");
		}
	}

	// Wheel bounds가 current radius/width measurement에 사용할 수 있는 finite non-zero dimension인지 확인합니다.
	bool HasUsableWheelBounds(const FCFVehicleWheelAssetSnapshot& WheelSnapshot)
	{
		if (!WheelSnapshot.bAssetLoaded)
		{
			return false;
		}

		// 검사할 Wheel local bounds extent입니다.
		const FVector& BoundsExtent = WheelSnapshot.BoundsExtent;
		// Radius 후보가 사용하는 X/Z 중 큰 절대 extent입니다.
		const float RadiusExtent = FMath::Max(FMath::Abs(BoundsExtent.X), FMath::Abs(BoundsExtent.Z));
		// Width 후보가 사용하는 Y 절대 extent입니다.
		const float WidthExtent = FMath::Abs(BoundsExtent.Y);
		return FMath::IsFinite(BoundsExtent.X)
			&& FMath::IsFinite(BoundsExtent.Y)
			&& FMath::IsFinite(BoundsExtent.Z)
			&& RadiusExtent > 0.0f
			&& WidthExtent > 0.0f;
	}

	// SocketScaleFromChassis에 참여하는 Wheel Mesh가 canonical 100x25x100cm + centered bounds 계약인지 검사합니다.
	bool IsCanonicalSocketScaleWheelMesh(const FCFVehicleWheelAssetSnapshot& WheelSnapshot, FString& OutFailureReason)
	{
		if (!HasUsableWheelBounds(WheelSnapshot))
		{
			OutFailureReason = TEXT("Wheel bounds가 유효하지 않습니다.");
			return false;
		}

		return FCFWheelSizeUtils::ValidateCanonicalWheelBounds(
			WheelSnapshot.BoundsOrigin,
			WheelSnapshot.BoundsExtent,
			OutFailureReason);
	}

	// Recipe Wheel socket binding의 None을 current Project default 이름으로 해석합니다.
	FName ResolveWheelSocketName(const FName ConfiguredSocketName, const TCHAR* DefaultSocketName)
	{
		return ConfiguredSocketName.IsNone() ? FName(DefaultSocketName) : ConfiguredSocketName;
	}

	// FL/FR/RL/RR 순서의 effective Wheel socket 이름을 Resolver/AssetReader와 동일한 fallback 규칙으로 만듭니다.
	TArray<FName> BuildResolvedWheelSocketNames(const FCFVehicleAssetIntent& AssetIntent)
	{
		// 고정 4-role socket 이름입니다.
		TArray<FName> SocketNames;
		SocketNames.Reserve(4);
		SocketNames.Add(ResolveWheelSocketName(AssetIntent.BodyWheelSocketFL, TEXT("Wheel_Anchor_FL")));
		SocketNames.Add(ResolveWheelSocketName(AssetIntent.BodyWheelSocketFR, TEXT("Wheel_Anchor_FR")));
		SocketNames.Add(ResolveWheelSocketName(AssetIntent.BodyWheelSocketRL, TEXT("Wheel_Anchor_RL")));
		SocketNames.Add(ResolveWheelSocketName(AssetIntent.BodyWheelSocketRR, TEXT("Wheel_Anchor_RR")));
		return SocketNames;
	}

	// Standard Guided Hardpoint UI가 제공하는 physical location category를 deterministic 순서로 보존합니다.
	const TArray<FName>& StandardHardpointCategories()
	{
		static const TArray<FName> Categories =
		{
			TEXT("Top"),
			TEXT("Front"),
			TEXT("Back"),
			TEXT("LeftSide"),
			TEXT("RightSide"),
			TEXT("Bottom"),
			TEXT("Internal")
		};
		return Categories;
	}

	// Standard UI에서 허용하는 physical category인지 확인합니다.
	bool IsStandardHardpointCategory(const FName LocationCategory)
	{
		return StandardHardpointCategories().Contains(LocationCategory);
	}

	// <Category>_<NN> stable LocationSlotId에서 exact numeric suffix를 읽습니다.
	bool TryParseStandardHardpointIndex(const FName LocationSlotId, const FName LocationCategory, int32& OutIndex)
	{
		OutIndex = 0;
		if (LocationSlotId.IsNone() || LocationCategory.IsNone())
		{
			return false;
		}

		const FString Prefix = LocationCategory.ToString() + TEXT("_");
		const FString IdText = LocationSlotId.ToString();
		if (!IdText.StartsWith(Prefix, ESearchCase::CaseSensitive))
		{
			return false;
		}

		const FString NumberText = IdText.Mid(Prefix.Len());
		if (NumberText.IsEmpty() || !NumberText.IsNumeric())
		{
			return false;
		}

		OutIndex = FCString::Atoi(*NumberText);
		return OutIndex > 0;
	}

	// Standard Hardpoint의 creation-time HP_<LocationSlotId> Socket suggestion을 만듭니다.
	FName BuildStandardHardpointSocketName(const FName LocationSlotId)
	{
		return LocationSlotId.IsNone()
			? NAME_None
			: FName(*FString::Printf(TEXT("HP_%s"), *LocationSlotId.ToString()));
	}

	// LayoutCapture equality에서 임의 tolerance 없이 FVector component를 exact 비교합니다.
	bool AreVectorsExactlyEqual(const FVector& Left, const FVector& Right)
	{
		return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z;
	}

	// LayoutCapture equality에서 임의 tolerance/normalization 없이 FRotator component를 exact 비교합니다.
	bool AreRotatorsExactlyEqual(const FRotator& Left, const FRotator& Right)
	{
		return Left.Pitch == Right.Pitch && Left.Yaw == Right.Yaw && Left.Roll == Right.Roll;
	}

	// USER Reference review token을 저장하는 EditorPerProject config section prefix입니다.
	const TCHAR* ReferenceReviewConfigSection = TEXT("CarFight.VehicleBuilder.ReferenceReview");

	// RecipeId별 Reference review token section 이름을 만듭니다.
	FString BuildReferenceReviewSection(const FGuid& RecipeId)
	{
		return FString::Printf(TEXT("%s.%s"), ReferenceReviewConfigSection, *RecipeId.ToString(EGuidFormats::Digits));
	}

	// USER Driving acceptance token을 저장하는 EditorPerProject config section prefix입니다.
	const TCHAR* DrivingAcceptanceConfigSection = TEXT("CarFight.VehicleBuilder.DrivingAcceptance");

	// RecipeId별 USER Driving acceptance token section 이름을 만듭니다.
	FString BuildDrivingAcceptanceSection(const FGuid& RecipeId)
	{
		return FString::Printf(TEXT("%s.%s"), DrivingAcceptanceConfigSection, *RecipeId.ToString(EGuidFormats::Digits));
	}

	// PowerShell argument 하나를 안전한 double-quote 인자로 만듭니다.
	FString QuotePowerShellArgument(const FString& Value)
	{
		// Windows PowerShell quoted argument에서 embedded quote를 backslash가 아닌 doubled quote로 escape합니다.
		FString Escaped = Value;
		Escaped.ReplaceInline(TEXT("\""), TEXT("\"\""));
		return FString::Printf(TEXT("\"%s\""), *Escaped);
	}

	// Builder-generated companion 하나의 deterministic package/object identity를 만듭니다.
	FCFBuilderAssetIdentity BuildCompanionIdentity(const FString& PackageRoot, const FString& AssetName)
	{
		// Request에 넣을 exact companion identity입니다.
		FCFBuilderAssetIdentity Identity;
		Identity.PackageName = PackageRoot + TEXT("/") + AssetName;
		Identity.AssetName = FName(*AssetName);
		return Identity;
	}

	// Recipe asset name에서 companion naming용 16자 이하 stem을 만듭니다.
	FString BuildCompanionStem(const UCFVehicleRecipeData& Recipe)
	{
		// Recipe UObject 이름에서 시작한 semantic-free stem입니다.
		FString Stem = Recipe.GetName();
		Stem.RemoveFromStart(TEXT("DA_Recipe_"));
		Stem.RemoveFromStart(TEXT("DA_"));
		// package/object name에 안전한 문자만 남긴 stem입니다.
		FString SanitizedStem;
		SanitizedStem.Reserve(Stem.Len());
		for (const TCHAR Character : Stem)
		{
			if (FChar::IsAlnum(Character) || Character == TEXT('_'))
			{
				SanitizedStem.AppendChar(Character);
			}
		}
		if (SanitizedStem.IsEmpty())
		{
			SanitizedStem = TEXT("Vehicle");
		}
		return SanitizedStem.Left(16);
	}

	// Builder presentation Step의 stable identity와 현재 표시 제목을 한 곳에서 정의합니다.
	struct FVehicleBuilderStepDefinition
	{
		// 순서 변경과 무관하게 유지되는 semantic Step identity입니다.
		ECFVehicleBuilderStepId StepId;
		// USER-facing Step 제목입니다.
		const TCHAR* Title;
	};

	// 현재 Builder Step 순서/구성을 소유하는 단일 definition 목록을 반환합니다.
	TConstArrayView<FVehicleBuilderStepDefinition> GetVehicleBuilderStepDefinitions()
	{
		// 현재 P0 baseline Step 순서입니다. 새 Step은 이 목록에 추가하고 기존 ID는 semantic compatibility를 위해 재사용하지 않습니다.
		static const FVehicleBuilderStepDefinition StepDefinitions[] =
		{
			{ECFVehicleBuilderStepId::IdentityReference, TEXT("차량 / Reference")},
			{ECFVehicleBuilderStepId::MeshPrep, TEXT("Mesh 준비")},
			{ECFVehicleBuilderStepId::SocketGuide, TEXT("소켓 준비 / Naming")},
			{ECFVehicleBuilderStepId::LayoutCapture, TEXT("Layout Capture")},
			{ECFVehicleBuilderStepId::PhysicsProposal, TEXT("Physics Proposal")},
			{ECFVehicleBuilderStepId::GameplaySetup, TEXT("Gameplay Setup")},
			{ECFVehicleBuilderStepId::FinalReview, TEXT("Final Review")},
			{ECFVehicleBuilderStepId::DrivingTest, TEXT("Driving Test")}
		};
		return MakeArrayView(StepDefinitions);
	}
}

// Browser refresh 전에도 Stable Step 8개가 존재하도록 transient Builder 상태를 초기화합니다.
FCFVehicleBuilderVM::FCFVehicleBuilderVM()
{
	InitializeSteps();
	RebuildStepStates();
}

// Asset Registry 기반 차량/메시 후보 목록을 기존 Authoring VM으로 새로 읽습니다.
bool FCFVehicleBuilderVM::RefreshVehicles(FString& OutError)
{
	if (!AuthoringViewModel.IsValid())
	{
		OutError = TEXT("Builder 내부 Authoring ViewModel이 없습니다.");
		return false;
	}

	const bool bSucceeded = AuthoringViewModel->RefreshBrowser(FString(), OutError);
	if (!bSucceeded)
	{
		return false;
	}

	// Constructor에서 이미 생성된 Stable Step projection을 authoritative Browser truth로 다시 평가합니다.
	// 비정상적으로 비어 있더라도 기존 fail-safe를 유지해 presentation을 복구합니다.
	if (StepViews.IsEmpty())
	{
		InitializeSteps();
	}
	RebuildStepStates();
	return true;
}

// 목록 row 하나를 current Builder target으로 선택하고 fresh authoring context를 읽습니다.
bool FCFVehicleBuilderVM::SelectVehicle(const FCFVehicleListEntry& Entry, FString& OutError)
{
	// 실제 Browser row를 선택하면 explicit 신규 제작 진입 상태를 종료하고 selected vehicle workflow로 복귀합니다.
	ResetNewVehicleEntryState();

	if (!AuthoringViewModel.IsValid())
	{
		OutError = TEXT("Builder 내부 Authoring ViewModel이 없습니다.");
		return false;
	}

	bHasLoadedResearchDraft = false;
	LoadedResearchDraft = FCFBuilderResearchDraft();
	bHasLoadedPhysicsProposalDraft = false;
	LoadedPhysicsProposalDraft = FCFBuilderPhysicsDraft();
	GameplayGuidanceResult = FCFBuilderGameplayGuidanceResult();
	bHasGameplayGuidanceResult = false;
	FinalReviewResult = FCFBuilderFinalReviewResult();
	bHasFinalReviewResult = false;
	ClearPreparedFinalReviewApply();
	ClearFinalReviewUndoToken();
	DrivingBenchmarkResult = FCFVehicleBuilderBenchmarkResult();
	bHasDrivingBenchmarkResult = false;
	DrivingBenchmarkStateError.Reset();
	bUserTestDrivePreparedThisSession = false;
	ClearDrivingAcceptanceToken();
	CurrentReferenceEvidence.Reset();
	CurrentReferenceEvidencePath.Reset();
	ReferenceEvidenceStateError.Reset();
	AcceptedReferenceRecipeId.Invalidate();
	AcceptedReferenceEvidenceId.Invalidate();
	AcceptedReferenceEvidenceFingerprint.Reset();
	ClearPreparedResearchCompanion();
	ClearPreparedEvidenceRefresh();
	ClearPreparedPhysicsProposal();
	bHasCurrentResolveReadForStepDiagnostics = false;

	// Advanced Authoring VM의 full Preview가 Fresh인지 여부입니다. NewVehicle private Profile bootstrap 전에는 expected Resolver Blocked가 정상일 수 있습니다.
	const bool bAuthoringPreviewFresh = AuthoringViewModel->SelectVehicle(Entry, OutError);
	bHasCurrentResolveReadForStepDiagnostics = OutError.IsEmpty()
		&& !AuthoringViewModel->IsMeshOnlyCandidate()
		&& AuthoringViewModel->HasRecipe()
		&& AuthoringViewModel->GetResolveResult().Operation.Status == ECFAuthoringOpStatus::Succeeded;

	if (!AuthoringViewModel->IsMeshOnlyCandidate() && AuthoringViewModel->HasRecipe())
	{
		// 새 selection의 exact Evidence/resume state를 먼저 복원해 NewVehicle/CompleteExisting mode가 persistent Evidence를 누락하지 않게 합니다.
		FString ReferenceStateError;
		RefreshReferenceEvidenceState(ReferenceStateError);
		// 새 selection의 USER Driving local acceptance token을 mutation 없이 복원합니다.
		LoadDrivingAcceptanceToken();
	}

	if (!bAuthoringPreviewFresh
		&& !CanUseNewVehicleProfileBootstrapRead()
		&& !CanUseHardpointSocketDraftRead())
	{
		if (OutError.IsEmpty())
		{
			OutError = BuildResolverFailureMessage(AuthoringViewModel->GetResolveResult());
			if (OutError.IsEmpty())
			{
				OutError = AuthoringViewModel->GetLastMessage().IsEmpty()
					? TEXT("현재 차량 Preview가 Blocked 상태이지만 상세 Resolver 오류가 비어 있습니다.")
					: AuthoringViewModel->GetLastMessage();
			}
		}
		RebuildStepStates();
		return false;
	}
	if (!bAuthoringPreviewFresh)
	{
		// Expected bootstrap Blocked는 Builder workflow error가 아니며 prospective Companion Resolver/Validator가 실제 validity를 다시 증명합니다.
		OutError.Reset();
	}
	CurrentStepIndex = 0;
	RebuildStepStates();
	return true;
}

// 현재 선택과 authoritative truth를 다시 읽고 Step 상태를 재평가합니다.
bool FCFVehicleBuilderVM::RefreshCurrentState(FString& OutError)
{
	if (!AuthoringViewModel.IsValid())
	{
		OutError = TEXT("Builder 내부 Authoring ViewModel이 없습니다.");
		return false;
	}

	if (!AuthoringViewModel->HasSelection())
	{
		bHasCurrentResolveReadForStepDiagnostics = false;
		RebuildStepStates();
		return true;
	}

	// Manual refresh는 직전 USER dialog용 prepared Evidence/Profile/Final Apply approval을 폐기하고 fresh preview를 요구합니다.
	ClearPreparedEvidenceRefresh();
	ClearPreparedPhysicsProposal();
	ClearPreparedFinalReviewApply();

	if (!AuthoringViewModel->IsMeshOnlyCandidate() && AuthoringViewModel->HasRecipe())
	{
		bHasCurrentResolveReadForStepDiagnostics = false;
		// Advanced Authoring VM의 full Preview가 Fresh인지 여부입니다.
		const bool bAuthoringPreviewFresh = AuthoringViewModel->RefreshPreview(OutError);
		if (!bAuthoringPreviewFresh)
		{
			bHasCurrentResolveReadForStepDiagnostics = OutError.IsEmpty()
				&& AuthoringViewModel->GetResolveResult().Operation.Status == ECFAuthoringOpStatus::Succeeded;
			if (!CanUseNewVehicleProfileBootstrapRead() && !CanUseHardpointSocketDraftRead())
			{
				if (OutError.IsEmpty())
				{
					OutError = BuildResolverFailureMessage(AuthoringViewModel->GetResolveResult());
					if (OutError.IsEmpty())
					{
						OutError = AuthoringViewModel->GetLastMessage().IsEmpty()
							? TEXT("현재 차량 Preview가 Blocked 상태이지만 상세 Resolver 오류가 비어 있습니다.")
							: AuthoringViewModel->GetLastMessage();
					}
				}
				RebuildStepStates();
				return false;
			}
			// NewVehicle private Profile bootstrap 전의 expected Resolver Blocked는 Builder workflow error가 아닙니다.
			OutError.Reset();
		}
		bHasCurrentResolveReadForStepDiagnostics = AuthoringViewModel->GetResolveResult().Operation.Status == ECFAuthoringOpStatus::Succeeded;
		if (!RefreshReferenceEvidenceState(OutError))
		{
			RebuildStepStates();
			return false;
		}
	}

	RebuildStepStates();
	return true;
}

// 기존 Browser selection과 독립적인 신규 차량 제작 진입 상태를 시작합니다.
void FCFVehicleBuilderVM::BeginNewVehicleEntry()
{
	// 이전 managed/mesh Browser selection이 refresh 뒤 다시 살아나 New Vehicle mode를 해제하지 않도록 Authoring selection 자체를 끊습니다.
	if (AuthoringViewModel.IsValid())
	{
		AuthoringViewModel->ClearSelection();
	}

	ResetNewVehicleEntryState();
	NewVehicleEntryState.bActive = true;
	CurrentStepIndex = FMath::Max(0, FindStepIndexById(ECFVehicleBuilderStepId::IdentityReference));
	RebuildStepStates();
}

// 현재 Builder가 명시적 신규 차량 제작 진입 상태인지 반환합니다.
bool FCFVehicleBuilderVM::IsNewVehicleEntryActive() const
{
	return NewVehicleEntryState.bActive;
}

// 명시적 신규 차량을 Chassis 없는 Blank Start로 전환합니다.
void FCFVehicleBuilderVM::SetNewVehicleBlankStart()
{
	if (!NewVehicleEntryState.bActive)
	{
		return;
	}

	NewVehicleEntryState.StartMode = ENewVehicleStartMode::Blank;
	NewVehicleEntryState.OptionalChassisMeshPath.Reset();
	RebuildStepStates();
}

// 명시적 신규 차량의 optional Chassis StaticMesh exact object path를 설정합니다. Invalid/empty path는 Blank Start로 되돌립니다.
void FCFVehicleBuilderVM::SetNewVehicleChassisMeshPath(const FSoftObjectPath& ChassisMeshPath)
{
	if (!NewVehicleEntryState.bActive)
	{
		return;
	}

	if (!ChassisMeshPath.IsValid())
	{
		SetNewVehicleBlankStart();
		return;
	}

	NewVehicleEntryState.StartMode = ENewVehicleStartMode::ChassisMesh;
	NewVehicleEntryState.OptionalChassisMeshPath = ChassisMeshPath;
	RebuildStepStates();
}

// Explicit New Vehicle/Mesh Candidate Quick Start가 공유하는 Vehicle ID transient 입력을 저장하고 Unreal Asset-safe 규칙을 검증합니다.
bool FCFVehicleBuilderVM::SetVehicleCreationId(const FString& VehicleId, FString& OutError)
{
	NewVehicleEntryState.VehicleId = VehicleId;
	return ValidateVehicleCreationId(NewVehicleEntryState.VehicleId, OutError);
}

// 현재 Guided 신규 차량 creation Vehicle ID transient 입력을 반환합니다.
const FString& FCFVehicleBuilderVM::GetVehicleCreationId() const
{
	return NewVehicleEntryState.VehicleId;
}

// Vehicle ID 하나에서 canonical default Definition/Recipe package/object identity 네 값을 deterministic하게 만듭니다.
bool FCFVehicleBuilderVM::BuildDefaultVehicleRecordIdentity(
	const FString& VehicleId,
	FString& OutDefinitionPackageName,
	FString& OutDefinitionAssetName,
	FString& OutRecipePackageName,
	FString& OutRecipeAssetName,
	FString& OutError) const
{
	OutDefinitionPackageName.Reset();
	OutDefinitionAssetName.Reset();
	OutRecipePackageName.Reset();
	OutRecipeAssetName.Reset();

	if (!ValidateVehicleCreationId(VehicleId, OutError))
	{
		return false;
	}

	// 일반 Guided 신규 차량 identity가 위치하는 canonical Authoring package root입니다.
	const FString AuthoringPackageRoot = TEXT("/Game/CarFight/Data/Authoring/");
	// VehicleData object/package에 사용할 deterministic 이름입니다.
	OutDefinitionAssetName = TEXT("DA_Vehicle_") + VehicleId;
	// Recipe object/package에 사용할 deterministic 이름입니다.
	OutRecipeAssetName = TEXT("DA_Recipe_") + VehicleId;
	OutDefinitionPackageName = AuthoringPackageRoot + OutDefinitionAssetName;
	OutRecipePackageName = AuthoringPackageRoot + OutRecipeAssetName;
	OutError.Reset();
	return true;
}

// 명시적 신규 차량이 현재 사용할 optional Chassis StaticMesh exact object path를 반환합니다.
const FSoftObjectPath& FCFVehicleBuilderVM::GetNewVehicleChassisMeshPath() const
{
	return NewVehicleEntryState.OptionalChassisMeshPath;
}

// Builder-owned 신규 차량 state로 기존 two-record 생성 proposal을 mutation0 준비합니다.
bool FCFVehicleBuilderVM::PrepareNewVehicleRecordCreate(
	const FString& DefinitionPackageName,
	const FString& DefinitionAssetName,
	const FString& RecipePackageName,
	const FString& RecipeAssetName,
	FCFVehicleRecordCreatePreview& OutPreview,
	FString& OutError)
{
	if (!IsNewVehicleEntryActive())
	{
		OutError = TEXT("먼저 '+ 새 차량 만들기'로 신규 차량 제작 모드에 들어가야 합니다.");
		return false;
	}

	return PrepareGuidedVehicleRecordCreate(
		DefinitionPackageName,
		DefinitionAssetName,
		RecipePackageName,
		RecipeAssetName,
		NewVehicleEntryState.OptionalChassisMeshPath,
		OutPreview,
		OutError);
}

// 직전 exact 신규 차량 proposal을 commit하고 record creation과 post-create exact Builder adoption 결과를 분리해 반환합니다.
bool FCFVehicleBuilderVM::ExecutePreparedNewVehicleRecordCreate(
	FCFVehicleRecordCreateResult& OutResult,
	bool& bOutBuilderAdopted,
	FString& OutAdoptionError,
	FString& OutError)
{
	bOutBuilderAdopted = false;
	OutAdoptionError.Reset();
	OutError.Reset();

	if (!AuthoringViewModel.IsValid())
	{
		OutError = TEXT("Builder 내부 Authoring ViewModel이 없습니다.");
		return false;
	}

	if (!AuthoringViewModel->ExecutePreparedVehicleCreate(OutResult))
	{
		OutError = OutResult.Operation.Message;
		return false;
	}

	// Record creation은 이미 성공했습니다. 이후 adoption 실패는 created Asset rollback 사유가 아니므로 별도 결과로 보존합니다.
	bOutBuilderAdopted = AdoptCreatedVehicleRecords(OutResult, OutAdoptionError);
	return true;
}

// 선택된 Mesh-only 후보에 대해 기존 two-record 생성 proposal을 mutation0으로 준비합니다.
bool FCFVehicleBuilderVM::PrepareSelectedMeshRecordCreate(
	const FString& DefinitionPackageName,
	const FString& DefinitionAssetName,
	const FString& RecipePackageName,
	const FString& RecipeAssetName,
	FCFVehicleRecordCreatePreview& OutPreview,
	FString& OutError)
{
	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->IsMeshOnlyCandidate())
	{
		OutError = TEXT("VehicleData가 없는 Mesh 후보를 먼저 선택해야 합니다.");
		return false;
	}

	return PrepareGuidedVehicleRecordCreate(
		DefinitionPackageName,
		DefinitionAssetName,
		RecipePackageName,
		RecipeAssetName,
		AuthoringViewModel->GetSelectedEntry().ChassisMeshPath,
		OutPreview,
		OutError);
}

// 직전 exact proposal에 explicit OwnershipWrite approval을 붙여 기존 two-record 생성 경로로 commit합니다.
bool FCFVehicleBuilderVM::ExecutePreparedMeshRecordCreate(FCFVehicleRecordCreateResult& OutResult, FString& OutError)
{
	// 기존 Quick Start caller도 신규 차량과 동일한 commit/adoption 경로를 재사용합니다.
	bool bBuilderAdopted = false;
	// Record creation 이후 Builder adoption 실패 진단입니다.
	FString AdoptionError;
	if (!ExecutePreparedNewVehicleRecordCreate(OutResult, bBuilderAdopted, AdoptionError, OutError))
	{
		return false;
	}

	if (!bBuilderAdopted)
	{
		OutError = FString::Printf(TEXT("Vehicle records는 생성됐지만 Builder adoption에 실패했습니다: %s"), *AdoptionError);
	}
	return true;
}

// Blank/Arbitrary/Mesh-only Quick Start가 공유하는 Guided two-record creation request를 기존 Authoring facade에 준비합니다.
bool FCFVehicleBuilderVM::PrepareGuidedVehicleRecordCreate(
	const FString& DefinitionPackageName,
	const FString& DefinitionAssetName,
	const FString& RecipePackageName,
	const FString& RecipeAssetName,
	const FSoftObjectPath& OptionalChassisMeshPath,
	FCFVehicleRecordCreatePreview& OutPreview,
	FString& OutError)
{
	if (!AuthoringViewModel.IsValid())
	{
		OutError = TEXT("Builder 내부 Authoring ViewModel이 없습니다.");
		return false;
	}

	// 모든 Guided 신규 차량이 공유하는 safe two-record 생성 요청입니다.
	FCFVehicleRecordCreateRequest Request;
	Request.DefinitionPackageName = DefinitionPackageName.TrimStartAndEnd();
	Request.DefinitionAssetName = FName(*DefinitionAssetName.TrimStartAndEnd());
	Request.RecipePackageName = RecipePackageName.TrimStartAndEnd();
	Request.RecipeAssetName = FName(*RecipeAssetName.TrimStartAndEnd());
	if (OptionalChassisMeshPath.IsValid())
	{
		Request.ChassisMesh = TSoftObjectPtr<UStaticMesh>(OptionalChassisMeshPath);
	}
	Request.bRequireVehicleSpecificTransmission = true;
	Request.bRequireExplicitHardpointPlan = true;
	// ProfileBindings는 의도적으로 비워 둡니다. Reference/물리/차급은 후속 Builder 단계가 소유합니다.
	if (!AuthoringViewModel->PrepareVehicleRecordCreate(Request, OutPreview))
	{
		OutError = AuthoringViewModel->GetLastMessage();
		return false;
	}

	OutError.Reset();
	return true;
}

// 생성된 exact Definition/Recipe를 Browser fresh row로 다시 찾아 Builder current target으로 adoption합니다.
bool FCFVehicleBuilderVM::AdoptCreatedVehicleRecords(
	const FCFVehicleRecordCreateResult& CreateResult,
	FString& OutError)
{
	if (!CreateResult.CreatedDefinition || !CreateResult.CreatedRecipe)
	{
		OutError = TEXT("생성 결과에 exact Created Definition/Recipe가 없습니다.");
		return false;
	}

	// 생성된 unsaved VehicleData exact object path입니다.
	const FSoftObjectPath CreatedDefinitionPath(CreateResult.CreatedDefinition);
	// 생성된 unsaved Recipe exact object path입니다.
	const FSoftObjectPath CreatedRecipePath(CreateResult.CreatedRecipe);

	// Asset Registry refresh 전에도 loaded unsaved object로 선택 가능한 exact 생성 row입니다.
	FCFVehicleListEntry CreatedEntry;
	CreatedEntry.DefinitionPath = CreatedDefinitionPath;
	CreatedEntry.RecipePath = CreatedRecipePath;
	CreatedEntry.RecipeId = CreateResult.CreatedRecipe->RecipeId;
	CreatedEntry.ManageState = CreateResult.CreatedRecipe->ImportState.ManageState;

	// Builder의 NewVehicle bootstrap-aware selection 경로로 먼저 exact created target을 adoption합니다.
	FString SelectError;
	if (!SelectVehicle(CreatedEntry, SelectError))
	{
		OutError = FString::Printf(TEXT("records created / Builder adoption failed: created object selection 실패: %s"), *SelectError);
		return false;
	}

	// AssetCreated 이후 Browser를 fresh read해 실제 Browser row가 exact created identity로 나타나는지 검증합니다.
	FString RefreshError;
	if (!RefreshVehicles(RefreshError))
	{
		OutError = FString::Printf(TEXT("records created / Builder adoption failed: Browser refresh 실패: %s"), *RefreshError);
		return false;
	}

	// Fresh Browser에서 exact Created Definition+Recipe를 동시에 가리키는 row만 adoption 후보로 인정합니다.
	const FCFVehicleListEntry* FreshCreatedEntry = GetVehicleEntries().FindByPredicate(
		[&CreatedDefinitionPath, &CreatedRecipePath](const FCFVehicleListEntry& Entry)
		{
			return Entry.DefinitionPath == CreatedDefinitionPath && Entry.RecipePath == CreatedRecipePath;
		});
	if (!FreshCreatedEntry)
	{
		OutError = TEXT("records created / Builder adoption failed: fresh Browser에서 exact created Definition+Recipe row를 찾지 못했습니다.");
		return false;
	}

	// Browser authority의 exact fresh row로 selection을 한 번 더 정규화해 transient Builder state와 Step projection을 동기화합니다.
	if (!SelectVehicle(*FreshCreatedEntry, SelectError))
	{
		OutError = FString::Printf(TEXT("records created / Builder adoption failed: fresh Browser row selection 실패: %s"), *SelectError);
		return false;
	}

	const FCFVehicleListEntry& AdoptedEntry = GetSelectedEntry();
	if (AdoptedEntry.DefinitionPath != CreatedDefinitionPath || AdoptedEntry.RecipePath != CreatedRecipePath)
	{
		OutError = TEXT("records created / Builder adoption failed: current Builder selection identity가 created records와 다릅니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// Project Saved의 AI ResearchDraft JSON을 current Recipe/Target에 exact binding해 mutation 없이 읽습니다.
bool FCFVehicleBuilderVM::LoadResearchDraft(FString& OutError)
{
	// Draft가 binding되어야 할 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || IsMeshOnlyCandidate())
	{
		OutError = TEXT("Research Draft를 읽으려면 managed Recipe 차량을 먼저 선택해야 합니다.");
		return false;
	}

	// Current Recipe가 가리키는 exact Target VehicleData path입니다.
	const FSoftObjectPath CurrentTargetPath = Recipe->TargetVehicleData.ToSoftObjectPath();
	if (!Recipe->RecipeId.IsValid() || !CurrentTargetPath.IsValid())
	{
		OutError = TEXT("Current RecipeId 또는 Target VehicleData binding이 유효하지 않습니다.");
		return false;
	}

	// AI가 작성하는 transient Research Draft의 canonical Project Saved 경로입니다.
	const FString DraftPath = GetResearchDraftPath();
	// UTF-8 JSON 원문입니다.
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *DraftPath))
	{
		OutError = FString::Printf(TEXT("AI Research Draft를 찾을 수 없습니다: %s"), *DraftPath);
		return false;
	}

	// JSON에서 deserialize할 typed Draft입니다.
	FCFBuilderResearchDraft ParsedDraft;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonText, &ParsedDraft, 0, 0))
	{
		OutError = TEXT("ResearchDraft.json을 FCFBuilderResearchDraft schema로 읽을 수 없습니다.");
		return false;
	}
	if (ParsedDraft.SchemaRevision != 1)
	{
		OutError = FString::Printf(TEXT("지원하지 않는 Research Draft schema revision입니다: %d"), ParsedDraft.SchemaRevision);
		return false;
	}
	if (!ParsedDraft.RecipeId.IsValid() || ParsedDraft.RecipeId != Recipe->RecipeId)
	{
		OutError = TEXT("Research Draft의 RecipeId가 현재 선택한 managed Recipe와 일치하지 않습니다.");
		return false;
	}
	if (!ParsedDraft.TargetDefinitionPath.IsValid() || ParsedDraft.TargetDefinitionPath != CurrentTargetPath)
	{
		OutError = TEXT("Research Draft의 TargetDefinitionPath가 현재 Recipe Target과 일치하지 않습니다.");
		return false;
	}

	// Initial Research semantic validation에만 사용하는 transient Evidence validator입니다.
	UCFVehicleRefEvidence* EvidenceValidator = NewObject<UCFVehicleRefEvidence>(GetTransientPackage());
	if (!EvidenceValidator)
	{
		OutError = TEXT("Research Draft Evidence validator를 만들 수 없습니다.");
		return false;
	}
	if (!EvidenceValidator->ValidateInitialResearchPayload(ParsedDraft.EvidencePayload, OutError))
	{
		return false;
	}

	LoadedResearchDraft = MoveTemp(ParsedDraft);
	bHasLoadedResearchDraft = true;
	ClearPreparedResearchCompanion();
	ClearPreparedEvidenceRefresh();

	// Reference Research가 다시 로드되면 이미 준비된 Physics Proposal approval/draft를 현재 Evidence review보다 앞서 재사용하지 않습니다.
	bHasLoadedPhysicsProposalDraft = false;
	LoadedPhysicsProposalDraft = FCFBuilderPhysicsDraft();
	ClearPreparedPhysicsProposal();

	// Existing Evidence와 payload가 다르더라도 load 자체는 허용합니다. 동일성 검사는 Companion 보존 경로에서만 수행하고,
	// differing complete ResearchDraft는 별도 Reference Evidence Refresh R1 proposal의 입력으로 사용합니다.
	RebuildStepStates();
	OutError.Reset();
	return true;
}

// Loaded ResearchDraft와 current companion truth로 baseline-preserving R2 Companion proposal을 mutation 없이 준비합니다.
bool FCFVehicleBuilderVM::PrepareResearchCompanions(FCFBuilderCompanionPreview& OutPreview, FString& OutError)
{
	OutPreview = FCFBuilderCompanionPreview();
	ClearPreparedResearchCompanion();

	// Companion owner가 될 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || IsMeshOnlyCandidate())
	{
		OutError = TEXT("Companion을 준비하려면 managed Recipe 차량을 먼저 선택해야 합니다.");
		return false;
	}

	if (!RefreshCurrentState(OutError))
	{
		return false;
	}

	// Missing private Profile domain 수입니다.
	int32 MissingProfileCount = 0;
	if (!ReadPrivateProfileCompleteness(MissingProfileCount, OutError))
	{
		return false;
	}

	// Current Recipe에 exact binding된 persistent Evidence 존재 여부입니다.
	const bool bHasEvidence = CurrentReferenceEvidence.IsValid();
	if (bHasEvidence && MissingProfileCount == 0)
	{
		OutError = TEXT("Reference Evidence와 Builder-private 4 Profile companion이 이미 complete 상태입니다.");
		return false;
	}
	if ((!bHasEvidence || MissingProfileCount > 0) && !bHasLoadedResearchDraft)
	{
		OutError = FString::Printf(
			TEXT("AI Research Draft가 필요합니다. 먼저 '%s'를 준비한 뒤 'AI Research Draft 불러오기'를 실행하세요."),
			*GetResearchDraftPath());
		return false;
	}
	if (bHasEvidence && !ValidateDraftAgainstCurrentEvidence(OutError))
	{
		return false;
	}

	// Existing facade에 전달할 exact R2 Companion request입니다.
	FCFBuilderCompanionRequest Request;
	Request.Recipe = Recipe;
	Request.Mode = DeriveCompanionMode();
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ClientOperationId = FString::Printf(
		TEXT("VB-P0-09-ResearchCompanion-Preview-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));

	if (bHasEvidence)
	{
		Request.ExistingEvidencePath = CurrentReferenceEvidencePath;
	}
	else
	{
		Request.bHasInitialEvidencePayload = true;
		Request.NewEvidenceId = FGuid::NewGuid();
		Request.InitialEvidencePayload = LoadedResearchDraft.EvidencePayload;
	}

	if (MissingProfileCount > 0)
	{
		Request.bHasInitialProfilePayload = true;
		Request.InitialProfilePayload = LoadedResearchDraft.InitialProfilePayload;
	}

	FillCompanionAssetIdentities(Request);

	if (!FCFVehicleAuthoringService::PreviewBuilderCompanions(Request, OutPreview))
	{
		OutError = OutPreview.Operation.Message;
		return false;
	}

	PreparedResearchCompanionRequest = Request;
	PreparedResearchCompanionPreview = OutPreview;
	bHasPreparedResearchCompanion = true;
	OutError.Reset();
	return true;
}

// 직전 exact Companion proposal에 USER OwnershipWrite approval을 붙여 Evidence/Missing private Profiles를 commit합니다.
bool FCFVehicleBuilderVM::ExecutePreparedResearchCompanions(FCFBuilderCompanionResult& OutResult, FString& OutError)
{
	OutResult = FCFBuilderCompanionResult();
	if (!bHasPreparedResearchCompanion)
	{
		OutError = TEXT("먼저 current state에서 Companion 생성 내용을 검토해야 합니다.");
		return false;
	}

	// USER에게 직전에 보여 준 exact request를 fresh commit용으로 복사합니다.
	FCFBuilderCompanionRequest Request = PreparedResearchCompanionRequest;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ClientOperationId = FString::Printf(
		TEXT("VB-P0-09-ResearchCompanion-Commit-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	Request.CallContext.ApprovalScopeHash = PreparedResearchCompanionPreview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = PreparedResearchCompanionPreview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = PreparedResearchCompanionPreview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = PreparedResearchCompanionPreview.Proposal.ResolverContractRevision;

	// Prepared approval은 성공/실패 여부와 무관하게 one-shot으로 소비합니다.
	ClearPreparedResearchCompanion();

	if (!FCFVehicleAuthoringService::CreateBuilderCompanions(Request, OutResult))
	{
		OutError = OutResult.Operation.Message;
		RebuildStepStates();
		return false;
	}

	LoadedResearchDraft = FCFBuilderResearchDraft();
	bHasLoadedResearchDraft = false;

	// Companion binding 변화는 Step 5 private Profile proposal의 prerequisite identity를 바꿀 수 있으므로 기존 draft/approval을 폐기합니다.
	LoadedPhysicsProposalDraft = FCFBuilderPhysicsDraft();
	bHasLoadedPhysicsProposalDraft = false;
	ClearPreparedPhysicsProposal();

	if (AuthoringViewModel.IsValid() && !AuthoringViewModel->RefreshPreview(OutError))
	{
		RebuildStepStates();
		return false;
	}
	if (!RefreshReferenceEvidenceState(OutError))
	{
		RebuildStepStates();
		return false;
	}

	RebuildStepStates();
	OutError.Reset();
	return true;
}

// Loaded ResearchDraft를 current Existing Evidence complete replacement로 적용할 R1 proposal을 mutation 없이 준비합니다.
bool FCFVehicleBuilderVM::PrepareReferenceEvidenceRefresh(FCFBuilderEvidenceRefreshPreview& OutPreview, FString& OutError)
{
	OutPreview = FCFBuilderEvidenceRefreshPreview();
	ClearPreparedEvidenceRefresh();

	// Evidence Refresh owner가 될 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || IsMeshOnlyCandidate())
	{
		OutError = TEXT("Reference Evidence를 갱신하려면 managed Recipe 차량을 먼저 선택해야 합니다.");
		return false;
	}
	if (!CurrentReferenceEvidence.IsValid() || !CurrentReferenceEvidencePath.IsValid())
	{
		OutError = TEXT("갱신할 current existing Reference Evidence가 없습니다.");
		return false;
	}
	if (!bHasLoadedResearchDraft)
	{
		OutError = FString::Printf(
			TEXT("새 complete Research Draft가 필요합니다. 먼저 '%s'를 준비하고 'AI Research Draft 불러오기'를 실행하세요."),
			*GetResearchDraftPath());
		return false;
	}

	// Existing Evidence research replacement R1 request입니다.
	FCFBuilderEvidenceRefreshRequest Request;
	Request.Recipe = Recipe;
	Request.EvidencePath = CurrentReferenceEvidencePath;
	Request.ExpectedCurrentEvidenceFingerprint = CurrentReferenceEvidence->EvidenceFingerprint;
	Request.EvidencePayload = LoadedResearchDraft.EvidencePayload;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ClientOperationId = FString::Printf(
		TEXT("VB-P0-09-EvidenceRefresh-Preview-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));

	if (!FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(Request, OutPreview))
	{
		OutError = OutPreview.Operation.Message;
		return false;
	}

	PreparedEvidenceRefreshRequest = Request;
	PreparedEvidenceRefreshPreview = OutPreview;
	bHasPreparedEvidenceRefresh = true;
	OutError.Reset();
	return true;
}

// 직전 exact Evidence Refresh proposal에 USER AuthoringWrite approval을 붙여 existing Evidence research payload만 commit합니다.
bool FCFVehicleBuilderVM::ExecutePreparedEvidenceRefresh(FCFBuilderEvidenceRefreshResult& OutResult, FString& OutError)
{
	OutResult = FCFBuilderEvidenceRefreshResult();
	if (!bHasPreparedEvidenceRefresh)
	{
		OutError = TEXT("먼저 current state에서 Reference Evidence 갱신 내용을 검토해야 합니다.");
		return false;
	}

	// USER에게 직전에 보여 준 exact refresh request를 fresh commit용으로 복사합니다.
	FCFBuilderEvidenceRefreshRequest Request = PreparedEvidenceRefreshRequest;
	// USER가 승인한 exact mutation0 preview입니다.
	const FCFBuilderEvidenceRefreshPreview ApprovedPreview = PreparedEvidenceRefreshPreview;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ClientOperationId = FString::Printf(
		TEXT("VB-P0-09-EvidenceRefresh-Commit-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	Request.CallContext.ApprovalScopeHash = ApprovedPreview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = ApprovedPreview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = ApprovedPreview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = ApprovedPreview.Proposal.ResolverContractRevision;

	// Prepared approval은 성공/실패 여부와 무관하게 one-shot으로 소비합니다.
	ClearPreparedEvidenceRefresh();

	if (!FCFVehicleAuthoringService::CommitBuilderEvidenceRefresh(Request, ApprovedPreview, OutResult))
	{
		OutError = OutResult.Operation.Message;
		return false;
	}

	// Evidence fingerprint가 바뀌면 기존 PhysicsDraft/approval은 stale이므로 자동 재사용하지 않습니다.
	bHasLoadedPhysicsProposalDraft = false;
	LoadedPhysicsProposalDraft = FCFBuilderPhysicsDraft();
	ClearPreparedPhysicsProposal();
	ClearPreparedFinalReviewApply();

	// Same object identity의 CurrentReferenceEvidence는 commit된 새 fingerprint를 즉시 보므로 Step 1 review token/Step 5 receipt stale을 fresh 재평가합니다.
	RebuildStepStates();
	OutError.Reset();
	return true;
}

// Current Evidence fingerprint와 RecipeId를 USER-reviewed Reference token으로 local Editor settings에 기록합니다.
bool FCFVehicleBuilderVM::AcceptCurrentReferenceSet(FString& OutError)
{
	// USER review token의 current Recipe owner입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// USER가 실제 review할 current persistent Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Recipe || !Recipe->RecipeId.IsValid() || !Evidence)
	{
		OutError = TEXT("Reference Set 승인에는 valid managed Recipe와 current Reference Evidence가 필요합니다.");
		return false;
	}
	if (!ReferenceEvidenceStateError.IsEmpty())
	{
		OutError = ReferenceEvidenceStateError;
		return false;
	}
	if (HasBlockingReferenceConflict())
	{
		OutError = TEXT("Reference Evidence에 unresolved Block conflict 또는 ProposalBlock Unknown이 남아 있어 Reference Set을 승인할 수 없습니다.");
		return false;
	}

	// USER가 확인한 exact workflow navigation identity입니다.
	AcceptedReferenceRecipeId = Recipe->RecipeId;
	AcceptedReferenceEvidenceId = Evidence->EvidenceId;
	AcceptedReferenceEvidenceFingerprint = Evidence->EvidenceFingerprint;
	SaveReferenceReviewToken();
	RebuildStepStates();
	OutError.Reset();
	return true;
}

// AI가 작성해야 하는 current Project Saved ResearchDraft path를 반환합니다.
FString FCFVehicleBuilderVM::GetResearchDraftPath() const
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("ResearchDraft.json")));
}

// USER-facing Step 1 Reference identity/source/claim/conflict/unknown 요약을 만듭니다.
FString FCFVehicleBuilderVM::BuildReferenceSummary() const
{
	// Persistent Evidence가 있으면 그 truth를, 아직 없으면 loaded Draft를 표시합니다.
	const UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	// USER에게 보여줄 누적 summary입니다.
	FString Summary;

	if (!Evidence && !bHasLoadedResearchDraft)
	{
		return FString::Printf(
			TEXT("Reference Evidence 없음\nAI Research Draft: 아직 불러오지 않음\nDraft 경로: %s"),
			*GetResearchDraftPath());
	}

	// 표시할 Reference identity 목록입니다.
	const TArray<FCFRefVehicleIdentity>& ReferenceVehicles = Evidence
		? Evidence->ReferenceVehicles
		: LoadedResearchDraft.EvidencePayload.ReferenceVehicles;
	// 표시할 source 목록입니다.
	const TArray<FCFRefSourceCitation>& Sources = Evidence
		? Evidence->Sources
		: LoadedResearchDraft.EvidencePayload.Sources;
	// 표시할 claim 목록입니다.
	const TArray<FCFRefClaim>& Claims = Evidence
		? Evidence->Claims
		: LoadedResearchDraft.EvidencePayload.Claims;
	// 표시할 conflict 목록입니다.
	const TArray<FCFRefConflict>& Conflicts = Evidence
		? Evidence->Conflicts
		: LoadedResearchDraft.EvidencePayload.Conflicts;
	// 표시할 Unknown fact 목록입니다.
	const TArray<FCFRefUnknownFact>& UnknownFacts = Evidence
		? Evidence->UnknownFacts
		: LoadedResearchDraft.EvidencePayload.UnknownFacts;

	// Primary Reference identity입니다.
	const FCFRefVehicleIdentity* PrimaryReference = ReferenceVehicles.FindByPredicate([](const FCFRefVehicleIdentity& Identity)
	{
		return Identity.Role == ECFRefVehicleRole::Primary;
	});

	if (PrimaryReference)
	{
		Summary += FString::Printf(
			TEXT("Primary Reference: %s %s"),
			*PrimaryReference->Manufacturer,
			*PrimaryReference->Model);
		if (PrimaryReference->ModelYearStart > 0)
		{
			Summary += FString::Printf(TEXT(" | %d"), PrimaryReference->ModelYearStart);
		}
		if (!PrimaryReference->Trim.IsEmpty())
		{
			Summary += FString::Printf(TEXT(" | %s"), *PrimaryReference->Trim);
		}
		if (!PrimaryReference->Powertrain.IsEmpty())
		{
			Summary += FString::Printf(TEXT(" | %s"), *PrimaryReference->Powertrain);
		}
		if (!PrimaryReference->Transmission.IsEmpty())
		{
			Summary += FString::Printf(TEXT(" | %s"), *PrimaryReference->Transmission);
		}
		Summary += TEXT("\n");
	}

	// Canonical FACT/DERIVED claim 수입니다.
	int32 CanonicalClaimCount = 0;
	for (const FCFRefClaim& Claim : Claims)
	{
		if (Claim.ResolutionState == ECFRefClaimResolution::Canonical)
		{
			++CanonicalClaimCount;
		}
	}

	// unresolved Block conflict 수입니다.
	int32 BlockingConflictCount = 0;
	for (const FCFRefConflict& Conflict : Conflicts)
	{
		if (Conflict.Severity == ECFRefConflictSeverity::Block
			&& (Conflict.ResolutionPolicy == ECFRefResolutionPolicy::UnresolvedBlock || Conflict.ResolutionClaimId.IsNone()))
		{
			++BlockingConflictCount;
		}
	}

	// ProposalBlock Unknown 수입니다.
	int32 BlockingUnknownCount = 0;
	for (const FCFRefUnknownFact& UnknownFact : UnknownFacts)
	{
		if (UnknownFact.BlockingUse == ECFRefUnknownBlockingUse::ProposalBlock)
		{
			++BlockingUnknownCount;
		}
	}

	Summary += FString::Printf(
		TEXT("Reference %d | Source %d | Canonical Claim %d | Conflict %d | Unknown %d\nBlocking Conflict %d | ProposalBlock Unknown %d"),
		ReferenceVehicles.Num(),
		Sources.Num(),
		CanonicalClaimCount,
		Conflicts.Num(),
		UnknownFacts.Num(),
		BlockingConflictCount,
		BlockingUnknownCount);

	if (Evidence)
	{
		Summary += FString::Printf(
			TEXT("\nEvidence: %s\nEvidenceFingerprint: %s"),
			*CurrentReferenceEvidencePath.ToString(),
			*Evidence->EvidenceFingerprint);
	}
	else
	{
		Summary += FString::Printf(
			TEXT("\nAI Draft: %s\nPersistent Evidence는 아직 생성되지 않았습니다."),
			LoadedResearchDraft.ResearchLabel.IsEmpty() ? TEXT("<label 없음>") : *LoadedResearchDraft.ResearchLabel);
	}

	// Builder-private companion completeness diagnostic입니다.
	int32 MissingProfileCount = 0;
	// Profile owner validation diagnostic입니다.
	FString ProfileError;
	if (ReadPrivateProfileCompleteness(MissingProfileCount, ProfileError))
	{
		Summary += FString::Printf(TEXT("\nBuilder-private Profile: %d/4 ready"), 4 - MissingProfileCount);
	}
	else
	{
		Summary += FString::Printf(TEXT("\nBuilder-private Profile: BLOCKED — %s"), *ProfileError);
	}

	return Summary;
}

// AI가 작성해야 하는 current Project Saved PhysicsDraft path를 반환합니다.
FString FCFVehicleBuilderVM::GetPhysicsProposalDraftPath() const
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("PhysicsDraft.json")));
}

// Step 5가 요구하는 Step 1~4가 모두 current workflow forward-progress 조건을 만족하는지 검사합니다.
bool FCFVehicleBuilderVM::ArePhysicsProposalPrerequisitesComplete(FString& OutError) const
{
	// Step 5 전에 반드시 완료되어야 하는 semantic Step identity 목록입니다.
	const ECFVehicleBuilderStepId RequiredStepIds[] =
	{
		ECFVehicleBuilderStepId::IdentityReference,
		ECFVehicleBuilderStepId::MeshPrep,
		ECFVehicleBuilderStepId::SocketGuide,
		ECFVehicleBuilderStepId::LayoutCapture
	};

	for (const ECFVehicleBuilderStepId RequiredStepId : RequiredStepIds)
	{
		// Stable StepId로 찾은 current prerequisite projection입니다.
		const FCFVehicleBuilderStepView* RequiredStep = FindStepView(RequiredStepId);
		if (!RequiredStep || !IsStepSatisfiedForForwardProgress(*RequiredStep))
		{
			OutError = RequiredStep
				? FString::Printf(TEXT("Physics Proposal 전에 '%s' 단계가 forward-progress 조건을 만족해야 합니다. 현재 상태: %s"),
					*RequiredStep->Title.ToString(),
					StepStateText(RequiredStep->State))
				: TEXT("Physics Proposal prerequisite Step을 찾을 수 없습니다.");
			return false;
		}
	}

	OutError.Reset();
	return true;
}

// Current Recipe에 exact owner로 binding된 private 4 Profile path/data를 complete typed payload로 읽습니다.
bool FCFVehicleBuilderVM::BuildCurrentPrivateProfilePayload(
	FCFBuilderPrivateProfilePayload& OutPayload,
	FString& OutError) const
{
	OutPayload = FCFBuilderPrivateProfilePayload();

	// private Profile binding owner인 current Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !Recipe->RecipeId.IsValid())
	{
		OutError = TEXT("Builder-private Profile을 읽을 valid current Recipe가 없습니다.");
		return false;
	}

	// current private Profile completeness입니다.
	int32 MissingProfileCount = 0;
	if (!ReadPrivateProfileCompleteness(MissingProfileCount, OutError))
	{
		return false;
	}
	if (MissingProfileCount != 0)
	{
		OutError = FString::Printf(TEXT("Physics Proposal에는 Builder-private Profile 4/4가 필요합니다. Missing=%d."), MissingProfileCount);
		return false;
	}

	// current Recipe에 binding된 VehicleBase Profile입니다.
	UCFVehicleBaseProfile* VehicleBaseProfile = Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	// current Recipe에 binding된 Drivetrain Profile입니다.
	UCFDrivetrainProfile* DrivetrainProfile = Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	// current Recipe에 binding된 Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	// current Recipe에 binding된 Performance Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!VehicleBaseProfile || !DrivetrainProfile || !HandlingProfile || !PerformanceProfile)
	{
		OutError = TEXT("Builder-private 4 Profile을 current Recipe binding에서 load할 수 없습니다.");
		return false;
	}

	OutPayload.VehicleBaseProfilePath = FSoftObjectPath(VehicleBaseProfile);
	OutPayload.DrivetrainProfilePath = FSoftObjectPath(DrivetrainProfile);
	OutPayload.HandlingProfilePath = FSoftObjectPath(HandlingProfile);
	OutPayload.PerformanceProfilePath = FSoftObjectPath(PerformanceProfile);
	OutPayload.VehicleBaseData = VehicleBaseProfile->Data;
	OutPayload.DrivetrainData = DrivetrainProfile->Data;
	OutPayload.HandlingData = HandlingProfile->Data;
	OutPayload.PerformanceData = PerformanceProfile->Data;
	OutError.Reset();
	return true;
}

// Loaded PhysicsDraft를 current Recipe/Evidence/private Profile authority에 binding한 typed commit request로 만듭니다.
bool FCFVehicleBuilderVM::BuildPhysicsProposalRequest(
	FCFBuilderProfileCommitRequest& OutRequest,
	FString& OutError) const
{
	OutRequest = FCFBuilderProfileCommitRequest();

	if (!bHasLoadedPhysicsProposalDraft)
	{
		OutError = TEXT("AI Physics Proposal Draft를 먼저 불러와야 합니다.");
		return false;
	}

	// Physics Proposal owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// Step 1에서 exact discovery한 current Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Recipe || !Evidence)
	{
		OutError = TEXT("Physics Proposal에는 current managed Recipe와 accepted Reference Evidence가 필요합니다.");
		return false;
	}

	// Step 1 local token 또는 matching persistent receipt가 current Evidence approval provenance를 증명하는지 여부입니다.
	const bool bReferenceAccepted = IsCurrentReferenceAcceptedForProgress();
	if (!bReferenceAccepted)
	{
		OutError = TEXT("Current Reference Evidence가 USER-reviewed exact fingerprint 상태가 아닙니다. Step 1에서 Reference Set을 다시 확인하세요.");
		return false;
	}

	if ((LoadedPhysicsProposalDraft.SchemaRevision != 2 && LoadedPhysicsProposalDraft.SchemaRevision != 3)
		|| LoadedPhysicsProposalDraft.RecipeId != Recipe->RecipeId
		|| LoadedPhysicsProposalDraft.TargetDefinitionPath != Recipe->TargetVehicleData.ToSoftObjectPath()
		|| LoadedPhysicsProposalDraft.EvidenceId != Evidence->EvidenceId
		|| LoadedPhysicsProposalDraft.ExpectedEvidenceFingerprint != Evidence->EvidenceFingerprint)
	{
		OutError = TEXT("Loaded Physics Draft binding이 current Recipe/Target/Evidence와 달라졌습니다. AI Proposal을 fresh하게 다시 준비해야 합니다.");
		return false;
	}
	if (LoadedPhysicsProposalDraft.ConsumedClaimIds.IsEmpty())
	{
		OutError = TEXT("Physics Proposal이 소비한 canonical Evidence Claim ID가 없습니다.");
		return false;
	}
	if (LoadedPhysicsProposalDraft.ProposalCorrelationHash.IsEmpty())
	{
		OutError = TEXT("Physics Proposal correlation hash가 비어 있습니다.");
		return false;
	}

	// current exact private 4 Profile path/data입니다.
	FCFBuilderPrivateProfilePayload CurrentProfilePayload;
	if (!BuildCurrentPrivateProfilePayload(CurrentProfilePayload, OutError))
	{
		return false;
	}

	// AI Draft에 path가 명시됐다면 current exact private Profile binding과 반드시 일치해야 합니다.
	if ((LoadedPhysicsProposalDraft.ProfilePayload.VehicleBaseProfilePath.IsValid()
			&& LoadedPhysicsProposalDraft.ProfilePayload.VehicleBaseProfilePath != CurrentProfilePayload.VehicleBaseProfilePath)
		|| (LoadedPhysicsProposalDraft.ProfilePayload.DrivetrainProfilePath.IsValid()
			&& LoadedPhysicsProposalDraft.ProfilePayload.DrivetrainProfilePath != CurrentProfilePayload.DrivetrainProfilePath)
		|| (LoadedPhysicsProposalDraft.ProfilePayload.HandlingProfilePath.IsValid()
			&& LoadedPhysicsProposalDraft.ProfilePayload.HandlingProfilePath != CurrentProfilePayload.HandlingProfilePath)
		|| (LoadedPhysicsProposalDraft.ProfilePayload.PerformanceProfilePath.IsValid()
			&& LoadedPhysicsProposalDraft.ProfilePayload.PerformanceProfilePath != CurrentProfilePayload.PerformanceProfilePath))
	{
		OutError = TEXT("Physics Draft의 private Profile path가 current Recipe binding과 일치하지 않습니다. 다른 차량/구버전 Draft를 적용하지 않습니다.");
		return false;
	}

	// duplicate/None consumed Claim을 preview 전에 사람이 이해하기 쉬운 오류로 차단할 stable set입니다.
	TSet<FName> UniqueConsumedClaimIds;
	for (const FName ClaimId : LoadedPhysicsProposalDraft.ConsumedClaimIds)
	{
		if (ClaimId.IsNone() || UniqueConsumedClaimIds.Contains(ClaimId))
		{
			OutError = TEXT("Physics Proposal ConsumedClaimIds에 None 또는 중복 ID가 있습니다.");
			return false;
		}
		UniqueConsumedClaimIds.Add(ClaimId);
	}

	OutRequest.Recipe = Recipe;
	OutRequest.ExpectedOwnerRecipeId = Recipe->RecipeId;
	OutRequest.Payload = LoadedPhysicsProposalDraft.ProfilePayload;

	// 실제 mutation destination은 AI가 추측한 path가 아니라 current Recipe exact private binding으로 강제합니다.
	OutRequest.Payload.VehicleBaseProfilePath = CurrentProfilePayload.VehicleBaseProfilePath;
	OutRequest.Payload.DrivetrainProfilePath = CurrentProfilePayload.DrivetrainProfilePath;
	OutRequest.Payload.HandlingProfilePath = CurrentProfilePayload.HandlingProfilePath;
	OutRequest.Payload.PerformanceProfilePath = CurrentProfilePayload.PerformanceProfilePath;

	OutRequest.EvidenceBinding.EvidencePath = CurrentReferenceEvidencePath;
	OutRequest.EvidenceBinding.ExpectedEvidenceId = Evidence->EvidenceId;
	OutRequest.EvidenceBinding.ExpectedEvidenceFingerprint = Evidence->EvidenceFingerprint;
	OutRequest.EvidenceBinding.ConsumedClaimIds = LoadedPhysicsProposalDraft.ConsumedClaimIds;
	OutRequest.TransmissionReview = LoadedPhysicsProposalDraft.TransmissionReview;
	OutRequest.EngineCurveReview = LoadedPhysicsProposalDraft.EngineCurveReview;
	OutRequest.UpstreamBuilderProposalHash = LoadedPhysicsProposalDraft.ProposalCorrelationHash;
	OutRequest.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	OutError.Reset();
	return true;
}

// Current private Profile payload와 persistent Builder receipt를 existing PreviewBuilderProfiles로 fresh 검증할 read-only request를 만듭니다.
bool FCFVehicleBuilderVM::BuildCurrentPhysicsReceiptRequest(
	FCFBuilderProfileCommitRequest& OutRequest,
	FString& OutError) const
{
	OutRequest = FCFBuilderProfileCommitRequest();

	// receipt owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// receipt가 binding해야 하는 current Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Recipe || !Evidence || !Recipe->BuilderCommitReceipt.IsValid())
	{
		OutError = TEXT("Current Physics Proposal commit receipt가 없습니다.");
		return false;
	}

	// current private 4 Profile exact payload입니다.
	FCFBuilderPrivateProfilePayload CurrentProfilePayload;
	if (!BuildCurrentPrivateProfilePayload(CurrentProfilePayload, OutError))
	{
		return false;
	}

	OutRequest.Recipe = Recipe;
	OutRequest.ExpectedOwnerRecipeId = Recipe->RecipeId;
	OutRequest.Payload = CurrentProfilePayload;
	OutRequest.EvidenceBinding.EvidencePath = CurrentReferenceEvidencePath;
	OutRequest.EvidenceBinding.ExpectedEvidenceId = Evidence->EvidenceId;
	OutRequest.EvidenceBinding.ExpectedEvidenceFingerprint = Evidence->EvidenceFingerprint;
	OutRequest.EvidenceBinding.ConsumedClaimIds = Recipe->BuilderCommitReceipt.ConsumedClaimIds;
	OutRequest.TransmissionReview = Recipe->BuilderCommitReceipt.TransmissionReview;
	OutRequest.EngineCurveReview = Recipe->BuilderCommitReceipt.EngineCurveReview;

	// Resume 검증에서는 upstream opaque correlation 자체를 authorization으로 사용하지 않습니다. Existing facade가 receipt/evidence/profile/resolver를 fresh 비교합니다.
	OutRequest.UpstreamBuilderProposalHash = TEXT("GuidedBuilder.Step5.Resume.v1");
	OutRequest.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	OutError.Reset();
	return true;
}

// Project Saved의 AI PhysicsDraft JSON을 current Recipe/Target/accepted Evidence에 exact binding해 mutation 없이 읽습니다.
bool FCFVehicleBuilderVM::LoadPhysicsProposalDraft(FString& OutError)
{
	ClearPreparedPhysicsProposal();

	// Draft binding owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// Draft가 소비해야 하는 current accepted Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Recipe || !Evidence || IsMeshOnlyCandidate())
	{
		OutError = TEXT("Physics Proposal Draft를 읽으려면 managed Recipe와 current Reference Evidence가 필요합니다.");
		return false;
	}

	if (!ArePhysicsProposalPrerequisitesComplete(OutError))
	{
		return false;
	}

	// Step 1 local token 또는 matching persistent receipt가 current Evidence approval provenance를 증명하는지 여부입니다.
	const bool bReferenceAccepted = IsCurrentReferenceAcceptedForProgress();
	if (!bReferenceAccepted)
	{
		OutError = TEXT("Physics Proposal 전에 Step 1 current Reference Set USER review가 필요합니다.");
		return false;
	}

	// AI가 작성하는 transient Physics Draft canonical Project Saved 경로입니다.
	const FString DraftPath = GetPhysicsProposalDraftPath();
	// UTF-8 JSON 원문입니다.
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *DraftPath))
	{
		OutError = FString::Printf(TEXT("AI Physics Proposal Draft를 찾을 수 없습니다: %s"), *DraftPath);
		return false;
	}

	// JSON에서 deserialize할 typed Physics Draft입니다.
	FCFBuilderPhysicsDraft ParsedDraft;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonText, &ParsedDraft, 0, 0))
	{
		OutError = TEXT("PhysicsDraft.json을 FCFBuilderPhysicsDraft schema로 읽을 수 없습니다.");
		return false;
	}
	if (ParsedDraft.SchemaRevision != 2 && ParsedDraft.SchemaRevision != 3)
	{
		OutError = FString::Printf(TEXT("지원하지 않는 Physics Draft schema revision입니다: %d. revision 2(Transmission) 또는 revision 3(Engine Curve review)가 필요합니다."), ParsedDraft.SchemaRevision);
		return false;
	}
	if (ParsedDraft.SchemaRevision == 2 && ParsedDraft.ProfilePayload.PerformanceData.bUseEngineTorqueCurve)
	{
		OutError = TEXT("Physics Draft revision 2는 EngineCurveReview가 없으므로 bUseEngineTorqueCurve=true payload를 사용할 수 없습니다. revision 3 Proposal이 필요합니다.");
		return false;
	}
	if (!ParsedDraft.RecipeId.IsValid() || ParsedDraft.RecipeId != Recipe->RecipeId)
	{
		OutError = TEXT("Physics Draft RecipeId가 current managed Recipe와 일치하지 않습니다.");
		return false;
	}
	if (!ParsedDraft.TargetDefinitionPath.IsValid()
		|| ParsedDraft.TargetDefinitionPath != Recipe->TargetVehicleData.ToSoftObjectPath())
	{
		OutError = TEXT("Physics Draft TargetDefinitionPath가 current Recipe Target과 일치하지 않습니다.");
		return false;
	}
	if (!ParsedDraft.EvidenceId.IsValid()
		|| ParsedDraft.EvidenceId != Evidence->EvidenceId
		|| ParsedDraft.ExpectedEvidenceFingerprint.IsEmpty()
		|| ParsedDraft.ExpectedEvidenceFingerprint != Evidence->EvidenceFingerprint)
	{
		OutError = TEXT("Physics Draft가 current accepted Reference Evidence identity/fingerprint와 일치하지 않습니다.");
		return false;
	}
	if (ParsedDraft.ProposalCorrelationHash.IsEmpty())
	{
		OutError = TEXT("Physics Draft ProposalCorrelationHash가 비어 있습니다.");
		return false;
	}
	if (ParsedDraft.ProposalLabel.IsEmpty() || ParsedDraft.UserFacingSummary.IsEmpty())
	{
		OutError = TEXT("Physics Draft에는 USER가 이해할 ProposalLabel과 UserFacingSummary가 필요합니다.");
		return false;
	}
	if (ParsedDraft.ConsumedClaimIds.IsEmpty())
	{
		OutError = TEXT("Physics Draft에는 실제로 소비한 canonical Evidence Claim ID가 최소 1개 필요합니다.");
		return false;
	}

	// current private 4 Profile binding이 실제로 complete/exact owner인지 확인하는 read-only payload입니다.
	FCFBuilderPrivateProfilePayload CurrentProfilePayload;
	if (!BuildCurrentPrivateProfilePayload(CurrentProfilePayload, OutError))
	{
		return false;
	}

	// AI가 path를 명시한 경우 다른 vehicle private Profile을 가리키지 않는지 미리 검사합니다.
	if ((ParsedDraft.ProfilePayload.VehicleBaseProfilePath.IsValid()
			&& ParsedDraft.ProfilePayload.VehicleBaseProfilePath != CurrentProfilePayload.VehicleBaseProfilePath)
		|| (ParsedDraft.ProfilePayload.DrivetrainProfilePath.IsValid()
			&& ParsedDraft.ProfilePayload.DrivetrainProfilePath != CurrentProfilePayload.DrivetrainProfilePath)
		|| (ParsedDraft.ProfilePayload.HandlingProfilePath.IsValid()
			&& ParsedDraft.ProfilePayload.HandlingProfilePath != CurrentProfilePayload.HandlingProfilePath)
		|| (ParsedDraft.ProfilePayload.PerformanceProfilePath.IsValid()
			&& ParsedDraft.ProfilePayload.PerformanceProfilePath != CurrentProfilePayload.PerformanceProfilePath))
	{
		OutError = TEXT("Physics Draft private Profile path가 current Recipe private binding과 다릅니다.");
		return false;
	}

	LoadedPhysicsProposalDraft = MoveTemp(ParsedDraft);
	bHasLoadedPhysicsProposalDraft = true;
	RebuildStepStates();
	OutError.Reset();
	return true;
}

// Loaded PhysicsDraft의 complete private 4 Profile payload를 existing typed facade로 mutation0 preview합니다.
bool FCFVehicleBuilderVM::PreparePhysicsProposal(
	FCFBuilderProfileCommitPreview& OutPreview,
	FString& OutError)
{
	OutPreview = FCFBuilderProfileCommitPreview();
	ClearPreparedPhysicsProposal();

	if (!RefreshCurrentState(OutError))
	{
		return false;
	}
	if (!ArePhysicsProposalPrerequisitesComplete(OutError))
	{
		return false;
	}

	// current binding을 적용한 exact typed Profile commit request입니다.
	FCFBuilderProfileCommitRequest Request;
	if (!BuildPhysicsProposalRequest(Request, OutError))
	{
		return false;
	}
	Request.CallContext.ClientOperationId = FString::Printf(
		TEXT("VB-P0-09-Physics-Preview-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));

	if (!FCFVehicleAuthoringService::PreviewBuilderProfiles(Request, OutPreview))
	{
		OutError = OutPreview.Operation.Message;
		return false;
	}

	PreparedPhysicsProposalRequest = Request;
	PreparedPhysicsProposalPreview = OutPreview;
	bHasPreparedPhysicsProposal = true;
	OutError.Reset();
	return true;
}

// 직전 exact Physics Proposal preview에 USER AuthoringWrite approval을 붙여 private 4 Profile + Builder receipt를 commit합니다.
bool FCFVehicleBuilderVM::ExecutePreparedPhysicsProposal(
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	OutResult = FCFAuthoringOpResult();
	if (!bHasPreparedPhysicsProposal)
	{
		OutError = TEXT("먼저 current Physics Proposal을 검토해야 합니다.");
		return false;
	}

	// USER에게 직전에 보여 준 exact typed request를 fresh commit용으로 복사합니다.
	FCFBuilderProfileCommitRequest Request = PreparedPhysicsProposalRequest;
	// USER가 승인한 exact mutation0 preview입니다.
	const FCFBuilderProfileCommitPreview ApprovedPreview = PreparedPhysicsProposalPreview;
	Request.ExpectedCurrentFingerprints = ApprovedPreview.CurrentFingerprints;
	Request.CallContext.ClientOperationId = FString::Printf(
		TEXT("VB-P0-09-Physics-Commit-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	Request.CallContext.ApprovalScopeHash = ApprovedPreview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = ApprovedPreview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = ApprovedPreview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = ApprovedPreview.Proposal.ResolverContractRevision;

	// Prepared approval은 성공/실패와 무관하게 one-shot으로 소비합니다.
	ClearPreparedPhysicsProposal();

	if (!FCFVehicleAuthoringService::CommitBuilderProfiles(Request, ApprovedPreview, OutResult))
	{
		OutError = OutResult.Message;
		RebuildStepStates();
		return false;
	}

	LoadedPhysicsProposalDraft = FCFBuilderPhysicsDraft();
	bHasLoadedPhysicsProposalDraft = false;

	if (AuthoringViewModel.IsValid() && !AuthoringViewModel->RefreshPreview(OutError))
	{
		RebuildStepStates();
		return false;
	}
	if (!RefreshReferenceEvidenceState(OutError))
	{
		RebuildStepStates();
		return false;
	}

	RebuildStepStates();
	OutError.Reset();
	return true;
}

// USER-facing Step 5 Proposal/receipt 상태 요약을 만듭니다.
FString FCFVehicleBuilderVM::BuildPhysicsProposalSummary() const
{
	// Step 5 current state를 함께 보여주기 위한 stable projection입니다.
	const FCFVehicleBuilderStepView* PhysicsStep = FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	// USER-facing 누적 summary입니다.
	FString Summary = PhysicsStep
		? PhysicsStep->Summary.ToString()
		: TEXT("Physics Proposal Step 상태를 찾을 수 없습니다.");

	// current managed Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (Recipe && Recipe->BuilderCommitReceipt.IsValid())
	{
		Summary += FString::Printf(
			TEXT("\nPersistent Builder receipt: 있음\nConsumed Claim: %d\nEvidenceFingerprint: %s\nEngine Curve Hash: %s"),
			Recipe->BuilderCommitReceipt.ConsumedClaimIds.Num(),
			*Recipe->BuilderCommitReceipt.EvidenceFingerprint,
			Recipe->BuilderCommitReceipt.EngineCurveProposalHash.IsEmpty() ? TEXT("<BaselineInherited>") : *Recipe->BuilderCommitReceipt.EngineCurveProposalHash);
	}
	else
	{
		Summary += TEXT("\nPersistent Builder receipt: 없음");
	}

	if (bHasLoadedPhysicsProposalDraft)
	{
		// USER가 읽기 쉬운 consumed Claim ID 문자열입니다.
		FString ClaimList;
		for (const FName ClaimId : LoadedPhysicsProposalDraft.ConsumedClaimIds)
		{
			if (!ClaimList.IsEmpty())
			{
				ClaimList += TEXT(", ");
			}
			ClaimList += ClaimId.ToString();
		}

		Summary += FString::Printf(
			TEXT("\n\nLoaded AI Proposal: %s\n%s\nConsumed Claim: %s\nDraft EvidenceFingerprint: %s\nEngine Curve: %s / Points=%d"),
			LoadedPhysicsProposalDraft.ProposalLabel.IsEmpty() ? TEXT("<label 없음>") : *LoadedPhysicsProposalDraft.ProposalLabel,
			*LoadedPhysicsProposalDraft.UserFacingSummary,
			ClaimList.IsEmpty() ? TEXT("<없음>") : *ClaimList,
			*LoadedPhysicsProposalDraft.ExpectedEvidenceFingerprint,
			LoadedPhysicsProposalDraft.ProfilePayload.PerformanceData.bUseEngineTorqueCurve ? TEXT("VehicleSpecific") : TEXT("BaselineInherited"),
			LoadedPhysicsProposalDraft.ProfilePayload.PerformanceData.EngineTorqueCurve.Points.Num());
	}
	else
	{
		Summary += FString::Printf(
			TEXT("\nAI Physics Draft: 아직 불러오지 않음\nDraft 경로: %s"),
			*GetPhysicsProposalDraftPath());
	}

	return Summary;
}

// Current Recipe에 binding된 Evidence를 Asset Registry에서 찾아 exact binding/fingerprint를 재검사합니다.
bool FCFVehicleBuilderVM::RefreshReferenceEvidenceState(FString& OutError)
{
	CurrentReferenceEvidence.Reset();
	CurrentReferenceEvidencePath.Reset();
	ReferenceEvidenceStateError.Reset();

	// Evidence binding owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		OutError = TEXT("Reference Evidence를 찾을 current Recipe가 없습니다.");
		ReferenceEvidenceStateError = OutError;
		return false;
	}
	// Current Recipe가 binding한 exact Target VehicleData입니다.
	UCFVehicleData* Target = Recipe->TargetVehicleData.LoadSynchronous();
	if (!Target)
	{
		OutError = TEXT("Reference Evidence binding을 검증할 Target VehicleData를 load할 수 없습니다.");
		ReferenceEvidenceStateError = OutError;
		return false;
	}

	// Current Recipe와 Target의 exact object path입니다.
	const FSoftObjectPath RecipePath(Recipe);
	// Current Target의 exact object path입니다.
	const FSoftObjectPath TargetPath(Target);

	// Project Asset Registry module입니다.
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// UCFVehicleRefEvidence exact/derived query를 수행할 registry입니다.
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	// Evidence class query filter입니다.
	FARFilter Filter;
	Filter.ClassPaths.Add(UCFVehicleRefEvidence::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	// Registry에서 찾은 모든 Reference Evidence asset입니다.
	TArray<FAssetData> EvidenceAssets;
	AssetRegistry.GetAssets(Filter, EvidenceAssets);

	// Current Recipe/Target과 exact binding이 일치하는 Evidence 목록입니다.
	TArray<UCFVehicleRefEvidence*> ExactMatches;
	for (const FAssetData& EvidenceAssetData : EvidenceAssets)
	{
		// Binding/fingerprint를 검사하기 위해 load한 Evidence입니다.
		UCFVehicleRefEvidence* Evidence = Cast<UCFVehicleRefEvidence>(EvidenceAssetData.GetAsset());
		if (!Evidence)
		{
			continue;
		}

		// Recipe GUID ownership이 current Recipe와 같은지 여부입니다.
		const bool bSameRecipeId = Evidence->TargetRecipeId == Recipe->RecipeId;
		// Recipe object path ownership이 current Recipe와 같은지 여부입니다.
		const bool bSameRecipePath = Evidence->TargetRecipePath == RecipePath;
		if (!bSameRecipeId && !bSameRecipePath)
		{
			continue;
		}

		if (!bSameRecipeId || !bSameRecipePath || Evidence->TargetDefinitionPath != TargetPath)
		{
			OutError = FString::Printf(
				TEXT("Reference Evidence binding이 current Recipe/Target과 부분 일치하여 자동 선택할 수 없습니다: %s"),
				*FSoftObjectPath(Evidence).ToString());
			ReferenceEvidenceStateError = OutError;
			return false;
		}
		ExactMatches.Add(Evidence);
	}

	if (ExactMatches.Num() > 1)
	{
		OutError = FString::Printf(
			TEXT("같은 Recipe ID에 exact binding된 Reference Evidence가 %d개 발견됐습니다. 자동 선택하지 않습니다."),
			ExactMatches.Num());
		ReferenceEvidenceStateError = OutError;
		return false;
	}

	if (ExactMatches.Num() == 1)
	{
		// Current exact Evidence입니다.
		UCFVehicleRefEvidence* Evidence = ExactMatches[0];
		if (!Evidence->EvidenceId.IsValid())
		{
			OutError = TEXT("Current Reference Evidence의 EvidenceId가 유효하지 않습니다.");
			ReferenceEvidenceStateError = OutError;
			return false;
		}

		// Current semantic payload에서 fresh 계산한 fingerprint입니다.
		FString FreshFingerprint;
		if (!Evidence->BuildEvidenceFingerprint(FreshFingerprint, OutError))
		{
			ReferenceEvidenceStateError = OutError;
			return false;
		}
		if (FreshFingerprint != Evidence->EvidenceFingerprint)
		{
			OutError = TEXT("Current Reference Evidence의 generated fingerprint가 semantic payload와 일치하지 않습니다.");
			ReferenceEvidenceStateError = OutError;
			return false;
		}

		CurrentReferenceEvidence = Evidence;
		CurrentReferenceEvidencePath = FSoftObjectPath(Evidence);
	}

	LoadReferenceReviewToken();
	OutError.Reset();
	return true;
}

// Loaded Draft의 Evidence payload가 current existing Evidence와 semantic exact 동일한지 확인합니다.
bool FCFVehicleBuilderVM::ValidateDraftAgainstCurrentEvidence(FString& OutError) const
{
	if (!bHasLoadedResearchDraft || !CurrentReferenceEvidence.IsValid())
	{
		OutError.Reset();
		return true;
	}

	// Existing Evidence와 exact semantic fingerprint를 비교할 transient carrier입니다.
	UCFVehicleRefEvidence* DraftEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage());
	// Current persistent Evidence입니다.
	const UCFVehicleRefEvidence* ExistingEvidence = CurrentReferenceEvidence.Get();
	// Current Recipe owner입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!DraftEvidence || !ExistingEvidence || !Recipe)
	{
		OutError = TEXT("Loaded Research Draft와 current Evidence를 비교할 context가 없습니다.");
		return false;
	}

	DraftEvidence->EvidenceId = ExistingEvidence->EvidenceId;
	DraftEvidence->TargetRecipeId = Recipe->RecipeId;
	DraftEvidence->TargetRecipePath = FSoftObjectPath(Recipe);
	DraftEvidence->TargetDefinitionPath = Recipe->TargetVehicleData.ToSoftObjectPath();
	if (!DraftEvidence->ApplyInitialResearchPayload(LoadedResearchDraft.EvidencePayload, OutError))
	{
		return false;
	}
	if (DraftEvidence->EvidenceFingerprint != ExistingEvidence->EvidenceFingerprint)
	{
		OutError = TEXT("Loaded Research Draft가 current persistent Reference Evidence와 다릅니다. Existing Evidence를 Companion 경로에서 덮어쓰지 않습니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// Current Recipe가 companion-only baseline preservation을 요구하는 Existing 차량인지, pristine core NewVehicle인지 파생합니다.
ECFBuilderCompanionMode FCFVehicleBuilderVM::DeriveCompanionMode() const
{
	// Mode를 파생할 current managed Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		return ECFBuilderCompanionMode::CompleteExisting;
	}

	// Initial Import/Apply/Builder Profile commit이 이미 있었는지 여부입니다.
	// AuthoringRevision은 Mesh/WSA/Layout 같은 Recipe-only 사전 authoring에서도 증가하므로 NewVehicle lifecycle 전환 근거로 사용하지 않습니다.
	const bool bHasLifecycleHistory =
		!Recipe->ImportState.ImportedDefinitionHash.IsEmpty()
		|| !Recipe->AppliedState.AppliedDefinitionHash.IsEmpty()
		|| Recipe->BuilderCommitReceipt.IsValid();
	// Builder-private Profile binding이 하나라도 이미 존재하는지 여부입니다.
	const bool bHasAnyPrivateProfile =
		!Recipe->ProfileBindings.VehicleBaseProfile.IsNull()
		|| !Recipe->ProfileBindings.DrivetrainProfile.IsNull()
		|| !Recipe->ProfileBindings.HandlingProfile.IsNull()
		|| !Recipe->ProfileBindings.PerformanceProfile.IsNull();
	// Persistent Reference Evidence가 이미 존재하는지 여부입니다.
	const bool bHasEvidence = CurrentReferenceEvidence.IsValid();

	return (!bHasLifecycleHistory && !bHasAnyPrivateProfile && !bHasEvidence)
		? ECFBuilderCompanionMode::NewVehicle
		: ECFBuilderCompanionMode::CompleteExisting;
}

// NewVehicle가 필수 private Profile을 아직 만들지 않아 current Resolver만 Blocked인 bootstrap read인지 판정합니다.
bool FCFVehicleBuilderVM::CanUseNewVehicleProfileBootstrapRead() const
{
	if (!AuthoringViewModel.IsValid()
		|| !bHasCurrentResolveReadForStepDiagnostics
		|| AuthoringViewModel->IsMeshOnlyCandidate()
		|| !AuthoringViewModel->HasRecipe()
		|| !ReferenceEvidenceStateError.IsEmpty()
		|| DeriveCompanionMode() != ECFBuilderCompanionMode::NewVehicle)
	{
		return false;
	}

	// NewVehicle bootstrap이 실제로 해결해야 하는 private Profile 누락 수입니다.
	int32 MissingProfileCount = 0;
	// Shared/foreign owner 충돌은 bootstrap 허용 상태가 아니라 fail-closed blocker입니다.
	FString ProfileError;
	if (!ReadPrivateProfileCompleteness(MissingProfileCount, ProfileError) || MissingProfileCount <= 0)
	{
		return false;
	}

	// Read 자체는 성공했고 Resolver 결과만 Blocked여야 합니다. Error/실패 read는 bootstrap으로 완화하지 않습니다.
	const FCFVehicleResolveReadResult& ResolveRead = AuthoringViewModel->GetResolveResult();
	return ResolveRead.Operation.Status == ECFAuthoringOpStatus::Succeeded
		&& ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Blocked;
}

// UseHardpoints 작성 중 exact Socket 미생성으로 HardpointSocketMissing 계열 blocker만 존재하는 diagnostic read인지 판정합니다.
bool FCFVehicleBuilderVM::CanUseHardpointSocketDraftRead() const
{
	if (!AuthoringViewModel.IsValid()
		|| !bHasCurrentResolveReadForStepDiagnostics
		|| AuthoringViewModel->IsMeshOnlyCandidate()
		|| !AuthoringViewModel->HasRecipe())
	{
		return false;
	}

	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe
		|| Recipe->BuilderHardpointPlanMode != ECFBuilderHardpointPlanMode::UseHardpoints
		|| Recipe->HardpointIntents.IsEmpty())
	{
		return false;
	}

	const FCFVehicleResolveReadResult& ResolveRead = AuthoringViewModel->GetResolveResult();
	if (ResolveRead.Operation.Status != ECFAuthoringOpStatus::Succeeded
		|| ResolveRead.ResolveResult.ResolveStatus != ECFVehicleResolveStatus::Blocked)
	{
		return false;
	}

	// Hardpoint 작성 중 예상 blocker 외 Error/Blocked가 하나라도 섞이면 일반 fail-closed 경로를 유지합니다.
	int32 HardpointSocketMissingCount = 0;
	const auto IsAllowedIssueBucket = [&HardpointSocketMissingCount](const TArray<FCFVehicleValidationIssue>& Issues)
	{
		for (const FCFVehicleValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == ECFVehicleValidationSeverity::Error)
			{
				return false;
			}
			if (Issue.Severity != ECFVehicleValidationSeverity::Blocked)
			{
				continue;
			}

			if (Issue.IssueCode == TEXT("HardpointSocketMissing"))
			{
				++HardpointSocketMissingCount;
				continue;
			}
			if (Issue.IssueCode == TEXT("HardpointResolvedFieldMissing"))
			{
				const FString CanonicalPath = Issue.FieldPath.ToCanonicalString(true);
				if (CanonicalPath.EndsWith(TEXT(".LocalLocation")) || CanonicalPath.EndsWith(TEXT(".LocalRotation")))
				{
					continue;
				}
			}
			return false;
		}
		return true;
	};

	return IsAllowedIssueBucket(ResolveRead.ResolveResult.RecipeValidation)
		&& IsAllowedIssueBucket(ResolveRead.ResolveResult.ResolverValidation)
		&& IsAllowedIssueBucket(ResolveRead.ResolveResult.DefinitionValidation)
		&& HardpointSocketMissingCount > 0;
}

// Current Recipe name/RecipeId에서 deterministic missing companion asset identity 5종을 만듭니다.
void FCFVehicleBuilderVM::FillCompanionAssetIdentities(FCFBuilderCompanionRequest& InOutRequest) const
{
	// Companion naming owner인 current Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		return;
	}

	// Asset 이름에 사용할 semantic-free Recipe stem입니다.
	const FString Stem = BuildCompanionStem(*Recipe);
	// 동일 이름 Recipe끼리도 충돌하지 않도록 persistent RecipeId에서 파생한 8자 suffix입니다.
	const FString RecipeIdSuffix = Recipe->RecipeId.ToString(EGuidFormats::Digits).Left(8);
	// 동일 vehicle companion을 묶는 RecipeId-qualified package root입니다.
	const FString PackageRoot = FString::Printf(TEXT("/Game/CarFight/Data/Authoring/Builder/%s_%s"), *Stem, *RecipeIdSuffix);

	InOutRequest.EvidenceAsset = BuildCompanionIdentity(PackageRoot, TEXT("DA_Ref_") + Stem);
	InOutRequest.VehicleBaseAsset = BuildCompanionIdentity(PackageRoot, TEXT("DA_VB_") + Stem);
	InOutRequest.DrivetrainAsset = BuildCompanionIdentity(PackageRoot, TEXT("DA_DT_") + Stem);
	InOutRequest.HandlingAsset = BuildCompanionIdentity(PackageRoot, TEXT("DA_HDL_") + Stem);
	InOutRequest.PerformanceAsset = BuildCompanionIdentity(PackageRoot, TEXT("DA_PRF_") + Stem);
}

// Current Recipe의 private 4 Profile이 모두 exact OwnerRecipeId인지 검사하고 missing/foreign 상태를 구분합니다.
bool FCFVehicleBuilderVM::ReadPrivateProfileCompleteness(int32& OutMissingCount, FString& OutError) const
{
	OutMissingCount = 0;
	// Private Profile binding owner인 current Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		OutError = TEXT("Private Profile completeness를 읽을 current Recipe가 없습니다.");
		return false;
	}

	if (Recipe->ProfileBindings.VehicleBaseProfile.IsNull())
	{
		++OutMissingCount;
	}
	else
	{
		// Current VehicleBase Profile입니다.
		UCFVehicleBaseProfile* Profile = Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != Recipe->RecipeId)
		{
			OutError = TEXT("VehicleBase Profile이 load되지 않거나 Builder-private exact OwnerRecipeId가 아닙니다.");
			return false;
		}
	}

	if (Recipe->ProfileBindings.DrivetrainProfile.IsNull())
	{
		++OutMissingCount;
	}
	else
	{
		// Current Drivetrain Profile입니다.
		UCFDrivetrainProfile* Profile = Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != Recipe->RecipeId)
		{
			OutError = TEXT("Drivetrain Profile이 load되지 않거나 Builder-private exact OwnerRecipeId가 아닙니다.");
			return false;
		}
	}

	if (Recipe->ProfileBindings.HandlingProfile.IsNull())
	{
		++OutMissingCount;
	}
	else
	{
		// Current Handling Profile입니다.
		UCFHandlingProfile* Profile = Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != Recipe->RecipeId)
		{
			OutError = TEXT("Handling Profile이 load되지 않거나 Builder-private exact OwnerRecipeId가 아닙니다.");
			return false;
		}
	}

	if (Recipe->ProfileBindings.PerformanceProfile.IsNull())
	{
		++OutMissingCount;
	}
	else
	{
		// Current Performance Profile입니다.
		UCFPerformanceProfile* Profile = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != Recipe->RecipeId)
		{
			OutError = TEXT("Performance Profile이 load되지 않거나 Builder-private exact OwnerRecipeId가 아닙니다.");
			return false;
		}
	}

	OutError.Reset();
	return true;
}

// Current Evidence의 unresolved Block conflict 또는 proposal-blocking Unknown이 존재하는지 반환합니다.
bool FCFVehicleBuilderVM::HasBlockingReferenceConflict() const
{
	// 검사할 current persistent Evidence입니다.
	const UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Evidence)
	{
		return false;
	}

	for (const FCFRefConflict& Conflict : Evidence->Conflicts)
	{
		if (Conflict.Severity == ECFRefConflictSeverity::Block
			&& (Conflict.ResolutionPolicy == ECFRefResolutionPolicy::UnresolvedBlock || Conflict.ResolutionClaimId.IsNone()))
		{
			return true;
		}
	}
	for (const FCFRefUnknownFact& UnknownFact : Evidence->UnknownFacts)
	{
		if (UnknownFact.BlockingUse == ECFRefUnknownBlockingUse::ProposalBlock)
		{
			return true;
		}
	}
	return false;
}

// Current local USER review token 또는 exact persistent Builder receipt가 current Reference Evidence 승인 provenance를 증명하는지 반환합니다.
bool FCFVehicleBuilderVM::IsCurrentReferenceAcceptedForProgress() const
{
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	const UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Recipe || !Evidence || !Recipe->RecipeId.IsValid() || !Evidence->EvidenceId.IsValid())
	{
		return false;
	}

	const bool bLocalTokenMatches =
		AcceptedReferenceRecipeId == Recipe->RecipeId
		&& AcceptedReferenceEvidenceId == Evidence->EvidenceId
		&& AcceptedReferenceEvidenceFingerprint == Evidence->EvidenceFingerprint;
	if (bLocalTokenMatches)
	{
		return true;
	}

	// Physics Proposal commit은 Step 1 explicit USER review를 prerequisite로 가지므로 matching persistent receipt는 Editor restart 후 durable acceptance provenance입니다.
	const FCFVehicleBuilderCommitReceipt& Receipt = Recipe->BuilderCommitReceipt;
	if (!Receipt.IsValid())
	{
		return false;
	}

	// Legacy receipt가 path를 기록하지 않은 경우에도 exact EvidenceId/Fingerprint가 동일하면 acceptance provenance는 복원할 수 있습니다.
	const bool bPathMatches = !Receipt.EvidencePath.IsValid() || Receipt.EvidencePath == CurrentReferenceEvidencePath;
	return bPathMatches
		&& Receipt.EvidenceId == Evidence->EvidenceId
		&& Receipt.EvidenceFingerprint == Evidence->EvidenceFingerprint;
}

// Current RecipeId에 저장된 USER Reference review token을 EditorPerProject settings에서 복원합니다.
void FCFVehicleBuilderVM::LoadReferenceReviewToken()
{
	AcceptedReferenceRecipeId.Invalidate();
	AcceptedReferenceEvidenceId.Invalidate();
	AcceptedReferenceEvidenceFingerprint.Reset();

	// Token owner인 current Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !Recipe->RecipeId.IsValid() || !GConfig)
	{
		return;
	}

	// Recipe별 local config section입니다.
	const FString Section = BuildReferenceReviewSection(Recipe->RecipeId);
	// Stored RecipeId 문자열입니다.
	FString RecipeIdText;
	// Stored EvidenceId 문자열입니다.
	FString EvidenceIdText;
	// Stored EvidenceFingerprint 문자열입니다.
	FString EvidenceFingerprint;
	if (!GConfig->GetString(*Section, TEXT("RecipeId"), RecipeIdText, GEditorPerProjectIni)
		|| !GConfig->GetString(*Section, TEXT("EvidenceId"), EvidenceIdText, GEditorPerProjectIni)
		|| !GConfig->GetString(*Section, TEXT("EvidenceFingerprint"), EvidenceFingerprint, GEditorPerProjectIni))
	{
		return;
	}

	// Parsed local RecipeId입니다.
	FGuid ParsedRecipeId;
	// Parsed local EvidenceId입니다.
	FGuid ParsedEvidenceId;
	if (!FGuid::Parse(RecipeIdText, ParsedRecipeId)
		|| !FGuid::Parse(EvidenceIdText, ParsedEvidenceId)
		|| ParsedRecipeId != Recipe->RecipeId
		|| EvidenceFingerprint.IsEmpty())
	{
		return;
	}

	AcceptedReferenceRecipeId = ParsedRecipeId;
	AcceptedReferenceEvidenceId = ParsedEvidenceId;
	AcceptedReferenceEvidenceFingerprint = MoveTemp(EvidenceFingerprint);
}

// Current USER Reference review token을 EditorPerProject settings에 저장합니다.
void FCFVehicleBuilderVM::SaveReferenceReviewToken() const
{
	if (!GConfig || !AcceptedReferenceRecipeId.IsValid() || !AcceptedReferenceEvidenceId.IsValid() || AcceptedReferenceEvidenceFingerprint.IsEmpty())
	{
		return;
	}

	// Recipe별 local config section입니다.
	const FString Section = BuildReferenceReviewSection(AcceptedReferenceRecipeId);
	// Stable RecipeId 문자열입니다.
	const FString RecipeIdText = AcceptedReferenceRecipeId.ToString(EGuidFormats::Digits);
	// Stable EvidenceId 문자열입니다.
	const FString EvidenceIdText = AcceptedReferenceEvidenceId.ToString(EGuidFormats::Digits);
	GConfig->SetString(*Section, TEXT("RecipeId"), *RecipeIdText, GEditorPerProjectIni);
	GConfig->SetString(*Section, TEXT("EvidenceId"), *EvidenceIdText, GEditorPerProjectIni);
	GConfig->SetString(*Section, TEXT("EvidenceFingerprint"), *AcceptedReferenceEvidenceFingerprint, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

// Selection 전환/새 draft load에서 이전 prepared Companion approval을 폐기합니다.
void FCFVehicleBuilderVM::ClearPreparedResearchCompanion()
{
	PreparedResearchCompanionRequest = FCFBuilderCompanionRequest();
	PreparedResearchCompanionPreview = FCFBuilderCompanionPreview();
	bHasPreparedResearchCompanion = false;
}

// Selection/refresh/draft 변경에서 이전 prepared Evidence Refresh approval을 폐기합니다.
void FCFVehicleBuilderVM::ClearPreparedEvidenceRefresh()
{
	PreparedEvidenceRefreshRequest = FCFBuilderEvidenceRefreshRequest();
	PreparedEvidenceRefreshPreview = FCFBuilderEvidenceRefreshPreview();
	bHasPreparedEvidenceRefresh = false;
}

// Selection/refresh/draft 변경에서 이전 prepared Physics Proposal approval을 폐기합니다.
void FCFVehicleBuilderVM::ClearPreparedPhysicsProposal()
{
	PreparedPhysicsProposalRequest = FCFBuilderProfileCommitRequest();
	PreparedPhysicsProposalPreview = FCFBuilderProfileCommitPreview();
	bHasPreparedPhysicsProposal = false;
}

// Selection/refresh에서 직전 Final Review DefinitionApply prepared approval을 폐기합니다.
void FCFVehicleBuilderVM::ClearPreparedFinalReviewApply()
{
	PreparedFinalReviewRequest = FCFBuilderFinalReviewRequest();
	PreparedFinalReviewResult = FCFBuilderFinalReviewResult();
	bHasPreparedFinalReviewApply = false;
}

// Selection이 바뀌거나 successful Undo 뒤 current lifetime guarded Undo token을 폐기합니다.
void FCFVehicleBuilderVM::ClearFinalReviewUndoToken()
{
	FinalReviewUndoToken = FCFBuilderUndoToken();
	bHasFinalReviewUndoToken = false;
}

// Step 하나가 현재 workflow에서 다음 단계 prerequisite를 만족하는지 공통 판정합니다.
bool FCFVehicleBuilderVM::IsStepSatisfiedForForwardProgress(const FCFVehicleBuilderStepView& Step) const
{
	if (Step.State == ECFVehicleBuilderStepState::Complete)
	{
		return true;
	}

	// LayoutCapture의 Ready는 current Socket truth가 유효하지만 Target Layout Apply가 Step 7로 defer된 NotCaptured 상태에서만 forward-progress를 허용합니다.
	if (Step.StepId == ECFVehicleBuilderStepId::LayoutCapture
		&& Step.State == ECFVehicleBuilderStepState::Ready
		&& AuthoringViewModel.IsValid())
	{
		const UCFVehicleData* TargetVehicleData = AuthoringViewModel->GetTargetVehicleData();
		return TargetVehicleData && !TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides;
	}

	return false;
}

// 현재 Step이 다음 단계로 진행 가능한지 반환합니다.
bool FCFVehicleBuilderVM::CanAdvanceFromCurrentStep() const
{
	return StepViews.IsValidIndex(CurrentStepIndex)
		&& StepViews.IsValidIndex(CurrentStepIndex + 1)
		&& IsStepSatisfiedForForwardProgress(StepViews[CurrentStepIndex]);
}

// Current page를 이전 Step으로 이동합니다.
void FCFVehicleBuilderVM::MovePreviousStep()
{
	CurrentStepIndex = FMath::Max(0, CurrentStepIndex - 1);
}

// Current Step이 workflow forward-progress 조건을 만족할 때 다음 Step으로 이동합니다.
bool FCFVehicleBuilderVM::MoveNextStep()
{
	if (!CanAdvanceFromCurrentStep())
	{
		return false;
	}

	++CurrentStepIndex;
	return true;
}

// USER가 상태를 확인할 목적으로 visible Step을 선택합니다. Locked/Unavailable은 이동하지 않습니다.
bool FCFVehicleBuilderVM::SelectVisibleStep(const int32 StepIndex)
{
	if (!StepViews.IsValidIndex(StepIndex))
	{
		return false;
	}

	const ECFVehicleBuilderStepState State = StepViews[StepIndex].State;
	if (State == ECFVehicleBuilderStepState::Locked || State == ECFVehicleBuilderStepState::Unavailable)
	{
		return false;
	}

	CurrentStepIndex = StepIndex;
	return true;
}

// 현재 Browser row입니다.
const TArray<FCFVehicleListEntry>& FCFVehicleBuilderVM::GetVehicleEntries() const
{
	static const TArray<FCFVehicleListEntry> EmptyEntries;
	return AuthoringViewModel.IsValid() ? AuthoringViewModel->GetBrowserEntries() : EmptyEntries;
}

// Stable StepId로 현재 projection을 찾습니다. presentation 순서 변경과 무관합니다.
const FCFVehicleBuilderStepView* FCFVehicleBuilderVM::FindStepView(const ECFVehicleBuilderStepId StepId) const
{
	// StepId가 현재 definition 목록에서 위치한 presentation index입니다.
	const int32 StepIndex = FindStepIndexById(StepId);
	return StepViews.IsValidIndex(StepIndex) ? &StepViews[StepIndex] : nullptr;
}

// 현재 Step projection입니다.
const FCFVehicleBuilderStepView& FCFVehicleBuilderVM::GetCurrentStep() const
{
	check(StepViews.IsValidIndex(CurrentStepIndex));
	return StepViews[CurrentStepIndex];
}

// 선택된 차량이 있는지 반환합니다.
bool FCFVehicleBuilderVM::HasSelection() const
{
	return AuthoringViewModel.IsValid() && AuthoringViewModel->HasSelection();
}

// 현재 선택 row를 반환합니다.
const FCFVehicleListEntry& FCFVehicleBuilderVM::GetSelectedEntry() const
{
	check(AuthoringViewModel.IsValid());
	return AuthoringViewModel->GetSelectedEntry();
}

// 현재 persistent Recipe를 반환합니다.
UCFVehicleRecipeData* FCFVehicleBuilderVM::GetRecipe() const
{
	return AuthoringViewModel.IsValid() ? AuthoringViewModel->GetRecipe() : nullptr;
}

// Current Recipe의 persistent Builder Hardpoint 계획 mode를 반환합니다. Recipe가 없으면 LegacyCompatible을 반환합니다.
ECFBuilderHardpointPlanMode FCFVehicleBuilderVM::GetHardpointPlanMode() const
{
	// Current managed Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	return Recipe ? Recipe->BuilderHardpointPlanMode : ECFBuilderHardpointPlanMode::LegacyCompatible;
}

// USER가 선택한 Guided Hardpoint 계획 mode를 Builder-owned Recipe transaction으로 기록하고 stale workflow approval을 폐기합니다.
bool FCFVehicleBuilderVM::CommitHardpointPlanMode(
	const ECFBuilderHardpointPlanMode PlanMode,
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	OutResult = FCFAuthoringOpResult();
	OutResult.OperationName = TEXT("SetBuilderHardpointPlanMode");
	OutResult.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
	OutResult.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;

	// Persistent Mode owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasRecipe() || !Recipe)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::RecipeNotFound;
		OutResult.Message = TEXT("Hardpoint 계획을 기록할 current managed Recipe가 없습니다.");
		OutError = OutResult.Message;
		return false;
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("LegacyCompatible은 기존 Recipe 호환 기본값이며 Guided USER 선택으로 전환할 수 없습니다.");
		OutError = OutResult.Message;
		return false;
	}

	if (PlanMode != ECFBuilderHardpointPlanMode::Unspecified
		&& PlanMode != ECFBuilderHardpointPlanMode::NoHardpoints
		&& PlanMode != ECFBuilderHardpointPlanMode::UseHardpoints)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("지원하지 않는 Builder Hardpoint 계획 mode입니다.");
		OutError = OutResult.Message;
		return false;
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::NoHardpoints
		&& (!Recipe->HardpointIntents.IsEmpty() || !Recipe->MountIntents.IsEmpty()))
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::DependencyConflict;
		OutResult.Message = TEXT("장착점 없음은 Hardpoint와 Mount가 모두 비어 있을 때만 선택할 수 있습니다. Mount를 먼저 제거하고 Hardpoint를 제거하세요.");
		OutError = OutResult.Message;
		return false;
	}

	if (Recipe->BuilderHardpointPlanMode == PlanMode)
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = TEXT("Builder Hardpoint 계획 mode가 이미 요청 상태와 같습니다.");
		OutResult.Mutation.bRecipeChanged = false;
		OutResult.Mutation.bTargetChanged = false;
		OutError.Reset();
		return true;
	}

	// 실패 시 exact 복원할 이전 persistent Mode입니다.
	const ECFBuilderHardpointPlanMode PreviousMode = Recipe->BuilderHardpointPlanMode;
	// 실패 시 exact 복원할 이전 diagnostic revision입니다.
	const int32 PreviousRevision = Recipe->AuthoringRevision;
	// 실패 시 exact 복원할 Recipe package dirty 상태입니다.
	UPackage* RecipePackage = Recipe->GetOutermost();
	// Transaction 전 package dirty 여부입니다.
	const bool bRecipePackageWasDirty = RecipePackage && RecipePackage->IsDirty();

	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "SetBuilderHardpointPlanMode", "차량 Builder 하드포인트 계획 변경"));
	Recipe->Modify();
	Recipe->BuilderHardpointPlanMode = PlanMode;
	Recipe->AuthoringRevision = PreviousRevision + 1;
	Recipe->MarkPackageDirty();
	Recipe->PostEditChange();

	if (Recipe->BuilderHardpointPlanMode != PlanMode || Recipe->AuthoringRevision != PreviousRevision + 1)
	{
		Recipe->BuilderHardpointPlanMode = PreviousMode;
		Recipe->AuthoringRevision = PreviousRevision;
		Recipe->PostEditChange();
		Transaction.Cancel();
		if (RecipePackage)
		{
			RecipePackage->SetDirtyFlag(bRecipePackageWasDirty);
		}
		OutResult.Status = ECFAuthoringOpStatus::FailedRolledBack;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = TEXT("Builder Hardpoint 계획 mode readback이 요청값과 달라 transaction을 rollback했습니다.");
		OutError = OutResult.Message;
		return false;
	}

	// Workflow metadata 변경으로 이전 Step 6/7 projection과 prepared DefinitionApply approval을 폐기합니다.
	GameplayGuidanceResult = FCFBuilderGameplayGuidanceResult();
	bHasGameplayGuidanceResult = false;
	FinalReviewResult = FCFBuilderFinalReviewResult();
	bHasFinalReviewResult = false;
	ClearPreparedFinalReviewApply();

	// 이미 완료된 DefinitionApply의 guarded Undo token은 별도 복구권한이므로 Mode metadata 변경만으로 폐기하지 않습니다.
	// Advanced Workspace 내부 prepared Apply도 stale하게 만들고 current semantic preview cache를 다시 읽습니다.
	FString PreviewRefreshError;
	AuthoringViewModel->RefreshPreview(PreviewRefreshError);
	bHasCurrentResolveReadForStepDiagnostics = AuthoringViewModel->GetResolveResult().Operation.Status == ECFAuthoringOpStatus::Succeeded;
	RebuildStepStates();

	OutResult.Status = ECFAuthoringOpStatus::Succeeded;
	OutResult.ErrorCode = ECFAuthoringErrorCode::None;
	OutResult.Message = TEXT("Builder Hardpoint 계획 mode를 Recipe에 기록했습니다. Target/StaticMesh/Save mutation은 없습니다.");
	OutResult.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bTargetChanged = false;
	OutResult.Mutation.bProfileChanged = false;
	OutResult.Mutation.bCreatedAssets = false;
	OutResult.Mutation.bPackageDirty = RecipePackage && RecipePackage->IsDirty();
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;
	OutError.Reset();
	return true;
}

// Exact MountProfile stable identity 하나를 existing R1 typed remove lane으로 제거하고 Builder state를 fresh 재평가합니다.
bool FCFVehicleBuilderVM::RemoveMountIntent(
	const FName MountProfileId,
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasRecipe())
	{
		OutError = TEXT("Mount를 제거할 current managed Recipe가 없습니다.");
		return false;
	}
	if (!AuthoringViewModel->RemoveMountIntent(MountProfileId, OutResult))
	{
		OutError = OutResult.Message;
		RebuildStepStates();
		return false;
	}
	if (!RefreshCurrentState(OutError))
	{
		return false;
	}
	OutError.Reset();
	return true;
}

// Standard Hardpoint 하나의 1:1 Mount rule을 complete draft로 검증하고 stable MountProfileId를 보존/생성해 existing typed R1 lane으로 commit합니다.
bool FCFVehicleBuilderVM::CommitStandardMountIntent(
	const FName LocationSlotId,
	const ECFVehicleMountType MountType,
	const ECFVehicleWeaponSize SizeLimit,
	const FSoftObjectPath& DefaultEquipmentPresetPath,
	FCFMountIntent& OutIntent,
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	OutIntent = FCFMountIntent();
	OutResult = FCFAuthoringOpResult();

	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasRecipe())
	{
		OutError = TEXT("Standard Mount를 반영할 current managed Recipe가 없습니다.");
		return false;
	}
	if (GetHardpointPlanMode() != ECFBuilderHardpointPlanMode::UseHardpoints)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("Standard Mount 반영 전에 Hardpoint Plan을 '장착 위치 사용'으로 명시해야 합니다.");
		OutError = OutResult.Message;
		return false;
	}
	if (LocationSlotId.IsNone())
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("Standard Mount에는 non-None Hardpoint LocationSlotId가 필요합니다.");
		OutError = OutResult.Message;
		return false;
	}
	if (MountType == ECFVehicleMountType::None)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("MountType을 Fixed/Gimbal/Turret/Launcher/Utility 중 하나로 선택하세요.");
		OutError = OutResult.Message;
		return false;
	}
	if (MountType != ECFVehicleMountType::Utility && SizeLimit == ECFVehicleWeaponSize::None)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("Fixed/Gimbal/Turret/Launcher Mount는 Small/Medium/Large SizeLimit이 필요합니다. Utility만 None을 허용합니다.");
		OutError = OutResult.Message;
		return false;
	}

	UCFVehicleRecipeData* Recipe = GetRecipe();
	const UCFVehicleData* TargetVehicleData = AuthoringViewModel->GetTargetVehicleData();
	const bool bHardpointExists = Recipe->HardpointIntents.ContainsByPredicate([LocationSlotId](const FCFHardpointIntent& Intent)
	{
		return Intent.LocationSlotId == LocationSlotId;
	});
	if (!bHardpointExists)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::DependencyConflict;
		OutResult.Message = FString::Printf(TEXT("Standard Mount가 참조할 Recipe Hardpoint가 없습니다: %s"), *LocationSlotId.ToString());
		OutError = OutResult.Message;
		return false;
	}

	TArray<const FCFMountIntent*> ExistingMountsForHardpoint;
	for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
	{
		if (MountIntent.LocationSlotRef == LocationSlotId)
		{
			ExistingMountsForHardpoint.Add(&MountIntent);
		}
	}
	if (ExistingMountsForHardpoint.Num() > 1)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::DependencyConflict;
		OutResult.Message = FString::Printf(
			TEXT("%s Hardpoint에 Recipe Mount가 %d개 연결돼 있습니다. Standard 1:1 편집은 multi-Mount를 자동 축소하지 않습니다. Advanced에서 구조를 먼저 정리하세요."),
			*LocationSlotId.ToString(),
			ExistingMountsForHardpoint.Num());
		OutError = OutResult.Message;
		return false;
	}

	FName StableMountProfileId = NAME_None;
	bool bExposedModule = true;
	if (ExistingMountsForHardpoint.Num() == 1)
	{
		const FCFMountIntent& ExistingMount = *ExistingMountsForHardpoint[0];
		if (ExistingMount.MountProfileId.IsNone())
		{
			OutResult.Status = ECFAuthoringOpStatus::Blocked;
			OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
			OutResult.Message = TEXT("Existing Standard candidate Mount의 MountProfileId가 None입니다. stable identity를 Advanced에서 먼저 교정하세요.");
			OutError = OutResult.Message;
			return false;
		}

		int32 SameIdCount = 0;
		for (const FCFMountIntent& CandidateMount : Recipe->MountIntents)
		{
			if (CandidateMount.MountProfileId == ExistingMount.MountProfileId)
			{
				++SameIdCount;
			}
		}
		if (SameIdCount != 1)
		{
			OutResult.Status = ECFAuthoringOpStatus::Blocked;
			OutResult.ErrorCode = ECFAuthoringErrorCode::DependencyConflict;
			OutResult.Message = FString::Printf(TEXT("Existing MountProfileId가 Recipe 안에서 중복됩니다: %s"), *ExistingMount.MountProfileId.ToString());
			OutError = OutResult.Message;
			return false;
		}

		StableMountProfileId = ExistingMount.MountProfileId;
		bExposedModule = ExistingMount.bExposedModule;
	}
	else
	{
		StableMountProfileId = FName(*FString::Printf(TEXT("Mount_%s"), *LocationSlotId.ToString()));

		const bool bRecipeIdCollision = Recipe->MountIntents.ContainsByPredicate([StableMountProfileId](const FCFMountIntent& Intent)
		{
			return Intent.MountProfileId == StableMountProfileId;
		});
		const bool bTargetIdCollision = TargetVehicleData && TargetVehicleData->MountProfiles.ContainsByPredicate([StableMountProfileId](const FCFVehicleMountProfile& Profile)
		{
			return Profile.MountProfileId == StableMountProfileId;
		});
		if (bRecipeIdCollision || bTargetIdCollision)
		{
			OutResult.Status = ECFAuthoringOpStatus::Blocked;
			OutResult.ErrorCode = ECFAuthoringErrorCode::DependencyConflict;
			OutResult.Message = FString::Printf(
				TEXT("새 Standard Mount identity가 current Recipe/Target과 충돌합니다: %s. 자동 suffix/rename하지 않습니다. 기존 identity ownership을 확인하세요."),
				*StableMountProfileId.ToString());
			OutError = OutResult.Message;
			return false;
		}
	}

	TSoftObjectPtr<UCFEquipmentPresetData> EquipmentPreset;
	if (DefaultEquipmentPresetPath.IsValid())
	{
		UObject* PresetObject = DefaultEquipmentPresetPath.ResolveObject();
		if (!PresetObject)
		{
			PresetObject = DefaultEquipmentPresetPath.TryLoad();
		}
		UCFEquipmentPresetData* PresetData = Cast<UCFEquipmentPresetData>(PresetObject);
		if (!PresetData)
		{
			OutResult.Status = ECFAuthoringOpStatus::Blocked;
			OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
			OutResult.Message = FString::Printf(TEXT("EquipmentPresetData를 기대 타입으로 읽을 수 없습니다: %s"), *DefaultEquipmentPresetPath.ToString());
			OutError = OutResult.Message;
			return false;
		}
		if (!PresetData->CanUseOnMount(MountType, SizeLimit))
		{
			OutResult.Status = ECFAuthoringOpStatus::Blocked;
			OutResult.ErrorCode = ECFAuthoringErrorCode::ValidationBlocked;
			OutResult.Message = FString::Printf(
				TEXT("선택한 EquipmentPresetData가 MountType/SizeLimit과 호환되지 않습니다: %s"),
				*DefaultEquipmentPresetPath.ToString());
			OutError = OutResult.Message;
			return false;
		}
		EquipmentPreset = TSoftObjectPtr<UCFEquipmentPresetData>(PresetData);
	}

	FCFMountIntent NewIntent;
	NewIntent.MountProfileId = StableMountProfileId;
	NewIntent.LocationSlotRef = LocationSlotId;
	NewIntent.MountType = MountType;
	NewIntent.SizeLimit = SizeLimit;
	NewIntent.DefaultEquipmentPresetData = EquipmentPreset;
	NewIntent.bExposedModule = bExposedModule;

	if (!AuthoringViewModel->UpsertMountIntent(NewIntent, OutResult))
	{
		OutError = OutResult.Message;
		RebuildStepStates();
		return false;
	}
	if (!RefreshCurrentState(OutError))
	{
		return false;
	}

	OutIntent = NewIntent;
	OutError.Reset();
	return true;
}

// Exact Hardpoint stable identity 하나를 dependency-safe existing R1 typed remove lane으로 제거하고 Builder state를 fresh 재평가합니다.
bool FCFVehicleBuilderVM::RemoveHardpointIntent(
	const FName LocationSlotId,
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasRecipe())
	{
		OutError = TEXT("Hardpoint를 제거할 current managed Recipe가 없습니다.");
		return false;
	}
	if (!AuthoringViewModel->RemoveHardpointIntent(LocationSlotId, OutResult))
	{
		OutError = OutResult.Message;
		RebuildStepStates();
		return false;
	}
	if (!RefreshCurrentState(OutError))
	{
		return false;
	}
	OutError.Reset();
	return true;
}

// Standard category 하나를 stable <Category>_<NN> / HP_<LocationSlotId> Hardpoint intent로 생성해 existing typed R1 lane으로 commit합니다.
bool FCFVehicleBuilderVM::AddStandardHardpoint(
	const FName LocationCategory,
	FCFHardpointIntent& OutIntent,
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	OutIntent = FCFHardpointIntent();
	OutResult = FCFAuthoringOpResult();

	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasRecipe())
	{
		OutError = TEXT("Standard Hardpoint를 추가할 current managed Recipe가 없습니다.");
		return false;
	}
	if (GetHardpointPlanMode() != ECFBuilderHardpointPlanMode::UseHardpoints)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = TEXT("Standard Hardpoint 추가 전에 '장착 위치 사용'을 명시적으로 선택해야 합니다.");
		OutError = OutResult.Message;
		return false;
	}
	if (!IsStandardHardpointCategory(LocationCategory))
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = FString::Printf(TEXT("Standard Guided 경로에서 지원하지 않는 위치 분류입니다: %s"), *LocationCategory.ToString());
		OutError = OutResult.Message;
		return false;
	}

	// Current Recipe와 relevant current Target의 stable LocationSlot identity를 함께 예약해 max-used+1을 계산합니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	const UCFVehicleData* TargetVehicleData = AuthoringViewModel->GetTargetVehicleData();
	int32 MaximumUsedIndex = 0;
	TSet<FName> ExistingLocationIds;
	for (const FCFHardpointIntent& ExistingIntent : Recipe->HardpointIntents)
	{
		ExistingLocationIds.Add(ExistingIntent.LocationSlotId);
		int32 ExistingIndex = 0;
		if (TryParseStandardHardpointIndex(ExistingIntent.LocationSlotId, LocationCategory, ExistingIndex))
		{
			MaximumUsedIndex = FMath::Max(MaximumUsedIndex, ExistingIndex);
		}
	}
	if (TargetVehicleData)
	{
		for (const FCFVehicleHardpointSlot& ExistingSlot : TargetVehicleData->HardpointSlots)
		{
			ExistingLocationIds.Add(ExistingSlot.LocationSlotId);
			int32 ExistingIndex = 0;
			if (TryParseStandardHardpointIndex(ExistingSlot.LocationSlotId, LocationCategory, ExistingIndex))
			{
				MaximumUsedIndex = FMath::Max(MaximumUsedIndex, ExistingIndex);
			}
		}
	}

	// 삭제된 번호를 재사용하지 않고 max-used+1부터 exact collision이 사라질 때까지 전진합니다.
	int32 CandidateIndex = MaximumUsedIndex + 1;
	FName CandidateLocationSlotId;
	do
	{
		CandidateLocationSlotId = FName(*FString::Printf(TEXT("%s_%02d"), *LocationCategory.ToString(), CandidateIndex));
		++CandidateIndex;
	}
	while (ExistingLocationIds.Contains(CandidateLocationSlotId));

	FCFHardpointIntent NewIntent;
	NewIntent.LocationSlotId = CandidateLocationSlotId;
	NewIntent.LocationCategory = LocationCategory;
	NewIntent.SocketName = BuildStandardHardpointSocketName(CandidateLocationSlotId);

	if (!AuthoringViewModel->UpsertHardpointIntent(NewIntent, OutResult))
	{
		OutError = OutResult.Message;
		RebuildStepStates();
		return false;
	}
	if (!RefreshCurrentState(OutError))
	{
		return false;
	}

	OutIntent = NewIntent;
	OutError.Reset();
	return true;
}

// Step 3 Standard UI가 노출하는 physical Hardpoint category 목록을 deterministic 순서로 반환합니다.
TArray<FName> FCFVehicleBuilderVM::GetStandardHardpointCategories() const
{
	return StandardHardpointCategories();
}

// Step 2의 USER 선택 Chassis/Wheel Mesh를 existing typed AssetIntent Recipe-only lane으로 반영하고 Builder state를 fresh 재평가합니다.
bool FCFVehicleBuilderVM::CommitMeshPreparation(
	const FCFVehicleAssetIntent& AssetIntent,
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasRecipe())
	{
		OutError = TEXT("Mesh 준비를 반영할 current managed Recipe가 없습니다.");
		return false;
	}

	if (!AuthoringViewModel->CommitAssetIntent(AssetIntent, OutResult))
	{
		OutError = OutResult.Message.IsEmpty()
			? TEXT("typed AssetIntent Recipe commit이 실패했습니다.")
			: OutResult.Message;
		RebuildStepStates();
		return false;
	}

	if (!RefreshCurrentState(OutError))
	{
		return false;
	}

	OutError.Reset();
	return true;
}

// 현재 선택이 사용하는 Chassis StaticMesh exact object path를 반환합니다.
FSoftObjectPath FCFVehicleBuilderVM::GetCurrentChassisMeshPath() const
{
	if (!AuthoringViewModel.IsValid() || !AuthoringViewModel->HasSelection())
	{
		return FSoftObjectPath();
	}

	if (AuthoringViewModel->IsMeshOnlyCandidate())
	{
		return AuthoringViewModel->GetSelectedEntry().ChassisMeshPath;
	}

	// Managed 차량의 current persistent Recipe입니다.
	const UCFVehicleRecipeData* Recipe = AuthoringViewModel->GetRecipe();
	return Recipe ? Recipe->AssetIntent.ChassisMesh.ToSoftObjectPath() : FSoftObjectPath();
}

// Step 3에서 반드시 준비해야 하는 effective FL/FR/RL/RR Wheel Socket 이름 4개를 current Recipe binding 기준으로 반환합니다.
TArray<FName> FCFVehicleBuilderVM::GetRequiredWheelSocketNames() const
{
	// Managed Recipe가 아직 없을 때도 USER가 표준 이름을 미리 확인할 수 있는 기본 binding입니다.
	const FName DefaultSocketNames[] = {TEXT("Wheel_Anchor_FL"), TEXT("Wheel_Anchor_FR"), TEXT("Wheel_Anchor_RL"), TEXT("Wheel_Anchor_RR")};
	// USER-facing required Wheel Socket 목록입니다.
	TArray<FName> RequiredSocketNames;

	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		RequiredSocketNames.Append(DefaultSocketNames, UE_ARRAY_COUNT(DefaultSocketNames));
		return RequiredSocketNames;
	}

	return BuildResolvedWheelSocketNames(Recipe->AssetIntent);
}

// Step 3에서 선택적으로 준비할 current Recipe Hardpoint/Destroyed FX Socket 이름을 중복 없이 반환합니다.
TArray<FName> FCFVehicleBuilderVM::GetOptionalSocketNames() const
{
	// Current Recipe가 실제로 요구하거나 사용할 수 있는 optional Socket 이름 집합입니다.
	TSet<FName> UniqueOptionalSocketNames;
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		return {};
	}

	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		// Explicit SocketName이 없으면 current standard HP_<LocationSlotId> 제안 이름을 사용합니다.
		const FName EffectiveHardpointSocketName = !HardpointIntent.SocketName.IsNone()
			? HardpointIntent.SocketName
			: (!HardpointIntent.LocationSlotId.IsNone()
				? FName(*FString::Printf(TEXT("HP_%s"), *HardpointIntent.LocationSlotId.ToString()))
				: NAME_None);
		if (!EffectiveHardpointSocketName.IsNone())
		{
			UniqueOptionalSocketNames.Add(EffectiveHardpointSocketName);
		}
	}

	if (Recipe->DefaultDataIntent.DestroyedFxMode != ECFAssetIntentMode::ExplicitNone)
	{
		// Recipe가 명시한 Destroyed FX Socket 또는 project default fallback 이름입니다.
		const FName DestroyedFxSocketName = Recipe->DefaultDataIntent.DestroyedFxSocketName.IsNone()
			? FName(TEXT("FX_Destroyed"))
			: Recipe->DefaultDataIntent.DestroyedFxSocketName;
		UniqueOptionalSocketNames.Add(DestroyedFxSocketName);
	}

	// USER에게 deterministic 순서로 보여줄 optional Socket 배열입니다.
	TArray<FName> OptionalSocketNames = UniqueOptionalSocketNames.Array();
	OptionalSocketNames.Sort([](const FName& LeftName, const FName& RightName)
	{
		return LeftName.LexicalLess(RightName);
	});
	return OptionalSocketNames;
}

// Step 3 Hardpoint 표와 중복되지 않는 Destroyed FX Socket 이름만 반환합니다.
TArray<FName> FCFVehicleBuilderVM::GetConditionalNonHardpointSocketNames() const
{
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		return {};
	}

	// Hardpoint 표가 별도로 표시하는 exact/effective Socket 이름입니다.
	TSet<FName> HardpointSocketNames;
	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		const FName EffectiveSocketName = !HardpointIntent.SocketName.IsNone()
			? HardpointIntent.SocketName
			: BuildStandardHardpointSocketName(HardpointIntent.LocationSlotId);
		if (!EffectiveSocketName.IsNone())
		{
			HardpointSocketNames.Add(EffectiveSocketName);
		}
	}

	TArray<FName> ConditionalNames;
	for (const FName SocketName : GetOptionalSocketNames())
	{
		if (!HardpointSocketNames.Contains(SocketName))
		{
			ConditionalNames.Add(SocketName);
		}
	}
	return ConditionalNames;
}

// Fresh AssetSnapshot에서 exact Chassis Socket이 현재 존재하는지 read-only로 반환합니다.
bool FCFVehicleBuilderVM::IsCurrentChassisSocketFound(const FName SocketName) const
{
	if (SocketName.IsNone() || !AuthoringViewModel.IsValid() || !AuthoringViewModel->IsPreviewFresh())
	{
		return false;
	}

	// Current fresh AssetSnapshot의 exact Chassis Socket fact입니다.
	const FCFVehicleSocketSnapshot* SocketFact = AuthoringViewModel->GetResolveResult().ResolveRequest.Assets.FindChassisSocket(SocketName);
	return SocketFact && SocketFact->bFound;
}

// 현재 선택이 아직 VehicleData가 없는 Mesh-only 후보인지 반환합니다.
bool FCFVehicleBuilderVM::IsMeshOnlyCandidate() const
{
	return AuthoringViewModel.IsValid() && AuthoringViewModel->IsMeshOnlyCandidate();
}

// Vehicle ID가 기본 naming에 사용할 영문자/숫자/_ 전용 Unreal Asset-safe identifier인지 검사합니다.
bool FCFVehicleBuilderVM::ValidateVehicleCreationId(const FString& VehicleId, FString& OutError) const
{
	if (VehicleId.IsEmpty())
	{
		OutError = TEXT("Vehicle ID를 입력하세요. 영문자, 숫자, _만 사용할 수 있습니다.");
		return false;
	}

	if (VehicleId != VehicleId.TrimStartAndEnd())
	{
		OutError = TEXT("Vehicle ID 앞뒤에는 공백을 사용할 수 없습니다. 영문자, 숫자, _만 사용하세요.");
		return false;
	}

	for (const TCHAR Character : VehicleId)
	{
		// Vehicle ID 기본 naming에서 허용하는 ASCII 영문자입니다.
		const bool bIsAsciiLetter = (Character >= TEXT('A') && Character <= TEXT('Z'))
			|| (Character >= TEXT('a') && Character <= TEXT('z'));
		// Vehicle ID 기본 naming에서 허용하는 ASCII 숫자입니다.
		const bool bIsAsciiDigit = Character >= TEXT('0') && Character <= TEXT('9');
		if (!bIsAsciiLetter && !bIsAsciiDigit && Character != TEXT('_'))
		{
			OutError = FString::Printf(
				TEXT("Vehicle ID에 사용할 수 없는 문자가 있습니다: '%c'. 영문자, 숫자, _만 사용하세요."),
				Character);
			return false;
		}
	}

	OutError.Reset();
	return true;
}

// 신규 차량 제작 transient state를 기본 비활성 상태로 되돌립니다.
void FCFVehicleBuilderVM::ResetNewVehicleEntryState()
{
	NewVehicleEntryState = FNewVehicleEntryState();
}

// 단일 Step Definition 목록에서 현재 presentation projection을 생성합니다.
void FCFVehicleBuilderVM::InitializeSteps()
{
	StepViews.Reset();

	// 현재 Builder Step 구성과 순서를 소유하는 단일 definition 목록입니다.
	const TConstArrayView<FVehicleBuilderStepDefinition> StepDefinitions = GetVehicleBuilderStepDefinitions();
	StepViews.Reserve(StepDefinitions.Num());
	for (int32 DefinitionIndex = 0; DefinitionIndex < StepDefinitions.Num(); ++DefinitionIndex)
	{
		// 현재 presentation에 추가할 definition입니다.
		const FVehicleBuilderStepDefinition& Definition = StepDefinitions[DefinitionIndex];
		// Slate가 표시할 transient Step projection입니다.
		FCFVehicleBuilderStepView& Step = StepViews.AddDefaulted_GetRef();
		Step.StepId = Definition.StepId;
		Step.StepNumber = DefinitionIndex + 1;
		Step.Title = FText::FromString(Definition.Title);
		Step.State = ECFVehicleBuilderStepState::Locked;
	}
}

// 현재 Authoring VM selection에서 각 Step evaluator를 순서대로 실행합니다.
void FCFVehicleBuilderVM::RebuildStepStates()
{
	if (StepViews.IsEmpty())
	{
		InitializeSteps();
	}

	for (FCFVehicleBuilderStepView& Step : StepViews)
	{
		Step.State = ECFVehicleBuilderStepState::Locked;
		Step.Summary = FText::FromString(FString::Printf(TEXT("%s 단계입니다."), *Step.Title.ToString()));
		Step.Resolution = FText::FromString(TEXT("앞 단계를 먼저 완료하세요."));
		Step.bProviderConnected = false;
	}

	EvaluateIdentityReferenceStep();
	if (IsNewVehicleEntryActive() || !HasSelection() || IsMeshOnlyCandidate() || !GetRecipe())
	{
		CurrentStepIndex = StepViews.IsEmpty() ? 0 : FMath::Clamp(CurrentStepIndex, 0, StepViews.Num() - 1);
		return;
	}

	// current Step Definition에서 파생된 presentation 순서대로 evaluator를 dispatch합니다.
	for (const FCFVehicleBuilderStepView& Step : StepViews)
	{
		if (Step.StepId == ECFVehicleBuilderStepId::IdentityReference)
		{
			continue;
		}
		EvaluateStepById(Step.StepId);
	}

	CurrentStepIndex = StepViews.IsEmpty() ? 0 : FMath::Clamp(CurrentStepIndex, 0, StepViews.Num() - 1);
}

// Stable StepId를 해당 Step evaluator로 dispatch합니다.
void FCFVehicleBuilderVM::EvaluateStepById(const ECFVehicleBuilderStepId StepId)
{
	switch (StepId)
	{
	case ECFVehicleBuilderStepId::IdentityReference:
		EvaluateIdentityReferenceStep();
		break;
	case ECFVehicleBuilderStepId::MeshPrep:
		EvaluateMeshPrepStep();
		break;
	case ECFVehicleBuilderStepId::SocketGuide:
		EvaluateSocketGuideStep();
		break;
	case ECFVehicleBuilderStepId::LayoutCapture:
		EvaluateLayoutCaptureStep();
		break;
	case ECFVehicleBuilderStepId::PhysicsProposal:
		EvaluatePhysicsProposalStep();
		break;
	case ECFVehicleBuilderStepId::GameplaySetup:
		EvaluateGameplaySetupStep();
		break;
	case ECFVehicleBuilderStepId::FinalReview:
		EvaluateFinalReviewStep();
		break;
	case ECFVehicleBuilderStepId::DrivingTest:
		EvaluateDrivingTestStep();
		break;
	default:
		break;
	}
}

// 차량/Reference 단계의 current state를 평가합니다.
void FCFVehicleBuilderVM::EvaluateIdentityReferenceStep()
{
	if (IsNewVehicleEntryActive())
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Ready,
			TEXT("새 차량 제작 시작 모드입니다. 아직 VehicleData/Recipe Asset은 생성하지 않았습니다."),
			TEXT("신규 차량 진입이 준비됐습니다. 후속 생성 UX에서 빈 차량 또는 Chassis Mesh 시작 방식과 Vehicle ID를 지정해 생성 검토를 이어갑니다."),
			true);
		CurrentStepIndex = FMath::Max(0, FindStepIndexById(ECFVehicleBuilderStepId::IdentityReference));
		return;
	}

	if (!HasSelection())
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Ready,
			TEXT("기존 차량/메시 후보를 선택하거나 새 차량 제작을 시작할 수 있습니다."),
			TEXT("기존 차량/메시 후보를 선택하거나 '+ 새 차량 만들기'로 새 차량 제작을 시작하세요."),
			true);
		CurrentStepIndex = FMath::Max(0, FindStepIndexById(ECFVehicleBuilderStepId::IdentityReference));
		return;
	}

	if (IsMeshOnlyCandidate())
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Ready,
			TEXT("선택한 항목은 아직 VehicleData가 없는 차체 메시 후보입니다."),
			TEXT("현재 Step의 '생성 내용 검토'에서 기존 safe two-record Preview를 확인한 뒤 명시적으로 승인하면 VehicleData+Recipe만 생성합니다. 자동 저장하지 않습니다."),
			true);
		return;
	}

	if (!GetRecipe())
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Blocked,
			TEXT("VehicleData는 있지만 Builder가 사용할 managed Recipe가 없습니다."),
			TEXT("기존 Initial Import/Record Create facade를 통해 Recipe를 만든 뒤 새로고침해야 합니다."),
			true);
		return;
	}

	// Step 1 completion의 persistent Recipe identity입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!ReferenceEvidenceStateError.IsEmpty())
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Blocked,
			ReferenceEvidenceStateError,
			TEXT("Reference Evidence binding/identity 충돌을 먼저 해결해야 합니다. 다른 Evidence를 임의로 자동 선택하지 않습니다."),
			true);
		return;
	}

	// Current Recipe에 exact binding된 persistent Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Evidence)
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Ready,
			bHasLoadedResearchDraft
				? TEXT("AI Research Draft가 current Recipe/Target에 exact binding되어 로드됐습니다. 아직 persistent Reference Evidence는 없습니다.")
				: TEXT("Managed 차량은 준비됐지만 persistent Reference Evidence가 아직 없습니다."),
			bHasLoadedResearchDraft
				? TEXT("'Companion 생성 내용 검토'에서 Evidence + Missing private Profile 생성 범위를 확인하고 명시적으로 승인하세요. 자동 저장/Apply는 하지 않습니다.")
				: FString::Printf(TEXT("AI가 %s 에 Research Draft를 준비한 뒤 'AI Research Draft 불러오기'를 실행하세요."), *GetResearchDraftPath()),
			true);
		return;
	}

	if (HasBlockingReferenceConflict())
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Blocked,
			TEXT("Current Reference Evidence에 unresolved Block conflict 또는 ProposalBlock Unknown이 남아 있습니다."),
			TEXT("AI Research에서 identity/source/conflict/required Unknown을 해결하고 새 reviewed Evidence state를 준비해야 합니다."),
			true);
		return;
	}

	// Local USER review token이 exact current Recipe/Evidence/fingerprint와 일치하는지 여부입니다.
	const bool bReviewTokenMatches =
		AcceptedReferenceRecipeId == Recipe->RecipeId
		&& AcceptedReferenceEvidenceId == Evidence->EvidenceId
		&& AcceptedReferenceEvidenceFingerprint == Evidence->EvidenceFingerprint;
	// 완료된 Physics Proposal의 persistent receipt는 해당 Evidence가 이미 USER-reviewed workflow를 통과했다는 durable provenance입니다.
	const FCFVehicleBuilderCommitReceipt& Receipt = Recipe->BuilderCommitReceipt;
	const bool bReceiptPathMatches = !Receipt.EvidencePath.IsValid() || Receipt.EvidencePath == CurrentReferenceEvidencePath;
	const bool bReceiptMatches = Receipt.IsValid()
		&& bReceiptPathMatches
		&& Receipt.EvidenceId == Evidence->EvidenceId
		&& Receipt.EvidenceFingerprint == Evidence->EvidenceFingerprint;
	if (bReviewTokenMatches || bReceiptMatches)
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Complete,
			bReviewTokenMatches
				? TEXT("Current Reference Evidence가 valid하고 local USER review token이 exact EvidenceFingerprint와 일치합니다.")
				: TEXT("Current Reference Evidence가 valid하고 persistent Builder receipt가 exact EvidenceId/Fingerprint를 증명해 완료 상태를 복원했습니다."),
			TEXT("Step 1 완료입니다. 이 provenance는 Reference review 완료만 증명하며 새 Profile/Recipe write, VehicleData Apply 또는 Save 권한을 주지 않습니다."),
			true);
		return;
	}

	// 같은 Recipe에 과거 Reference review token이 있었거나 durable receipt가 current Evidence와 달라졌으면 stale로 표시합니다.
	const bool bHasStaleReviewToken =
		AcceptedReferenceRecipeId == Recipe->RecipeId
		&& AcceptedReferenceEvidenceId.IsValid()
		&& !AcceptedReferenceEvidenceFingerprint.IsEmpty();
	const bool bHasStalePersistentReceipt = Receipt.IsValid() && !bReceiptMatches;
	if (bHasStaleReviewToken || bHasStalePersistentReceipt)
	{
		SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Stale,
			bHasStaleReviewToken
				? TEXT("과거 USER Reference review token이 current Evidence identity/fingerprint와 일치하지 않습니다.")
				: TEXT("Persistent Builder receipt의 accepted Evidence identity/fingerprint가 current Reference Evidence와 일치하지 않습니다."),
			TEXT("아래 Reference 요약을 다시 확인한 뒤 '이 Reference Set으로 진행'을 눌러 current fingerprint를 새로 승인하세요. 기존 receipt는 후속 Physics 단계에서 fresh 검증됩니다."),
			true);
		return;
	}

	SetStep(ECFVehicleBuilderStepId::IdentityReference, ECFVehicleBuilderStepState::Ready,
		TEXT("Current Reference Evidence가 valid하고 blocking condition이 없습니다. USER Reference Set 확인만 남았습니다."),
		TEXT("아래 Reference 요약을 확인한 뒤 '이 Reference Set으로 진행'을 누르세요. 필요한 Missing companion은 별도 'Companion 생성 내용 검토'에서 승인할 수 있습니다."),
		true);
}

// Mesh 준비 단계의 current state를 fresh AssetSnapshot에서 평가합니다.
void FCFVehicleBuilderVM::EvaluateMeshPrepStep()
{
	// MeshPrep이 읽을 current persistent Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		return;
	}

	if (!bHasCurrentResolveReadForStepDiagnostics)
	{
		SetStep(ECFVehicleBuilderStepId::MeshPrep, ECFVehicleBuilderStepState::Stale,
			TEXT("Mesh 상태를 판정할 current Resolve/AssetSnapshot read가 없습니다."),
			TEXT("'현재 상태 다시 확인'을 눌러 current AssetSnapshot을 다시 읽으세요."),
			true);
		return;
	}

	// Current Recipe의 persistent Asset intent입니다.
	const FCFVehicleAssetIntent& AssetIntent = Recipe->AssetIntent;
	// FCFVehicleAssetReader가 이번 fresh Resolve에서 만든 immutable asset truth입니다.
	const FCFVehicleAssetSnapshot& Assets = AuthoringViewModel->GetResolveResult().ResolveRequest.Assets;
	// MeshPrep hard blocker를 USER-facing 문구로 모읍니다.
	TArray<FString> MeshBlockers;
	// USER Wheel Socket Scale이 visual/physics authority인 신규 정상 mode입니다.
	const bool bSocketScaleMode = Recipe->WheelVisualIntent.Mode == ECFWheelVisualIntentMode::SocketScaleFromChassis;
	// Optional Wheel 누락을 current backend semantics대로 Warning으로 셉니다.
	int32 OptionalWheelMissingCount = 0;

	if (AssetIntent.ChassisMesh.IsNull())
	{
		MeshBlockers.Add(TEXT("Chassis Mesh가 지정되지 않았습니다."));
	}
	else if (!Assets.bChassisLoaded)
	{
		MeshBlockers.Add(FString::Printf(TEXT("Chassis Mesh를 resolve하지 못했습니다: %s"), *AssetIntent.ChassisMesh.ToSoftObjectPath().ToString()));
	}

	if (AssetIntent.WheelMeshFL.IsNull())
	{
		MeshBlockers.Add(TEXT("최소 기준 Wheel Mesh FL이 지정되지 않았습니다."));
	}
	else if (!Assets.WheelFL.bAssetLoaded)
	{
		MeshBlockers.Add(FString::Printf(TEXT("Wheel Mesh FL을 resolve하지 못했습니다: %s"), *AssetIntent.WheelMeshFL.ToSoftObjectPath().ToString()));
	}
	else if (!HasUsableWheelBounds(Assets.WheelFL))
	{
		MeshBlockers.Add(TEXT("Wheel Mesh FL bounds가 non-finite이거나 radius/width에 사용할 dimension이 0입니다."));
	}
	else if (bSocketScaleMode)
	{
		FString CanonicalFailure;
		if (!IsCanonicalSocketScaleWheelMesh(Assets.WheelFL, CanonicalFailure))
		{
			MeshBlockers.Add(FString::Printf(TEXT("Socket Scale mode의 Wheel Mesh FL이 canonical 규격을 만족하지 않습니다: %s"), *CanonicalFailure));
		}
	}

	// Optional FR/RL/RR role의 configured ref입니다.
	const TSoftObjectPtr<UStaticMesh> OptionalWheelRefs[] = {AssetIntent.WheelMeshFR, AssetIntent.WheelMeshRL, AssetIntent.WheelMeshRR};
	// Optional FR/RL/RR role의 fresh snapshot입니다.
	const FCFVehicleWheelAssetSnapshot* OptionalWheelSnapshots[] = {&Assets.WheelFR, &Assets.WheelRL, &Assets.WheelRR};
	// USER-facing optional role 이름입니다.
	const TCHAR* OptionalWheelRoleNames[] = {TEXT("FR"), TEXT("RL"), TEXT("RR")};
	for (int32 WheelIndex = 0; WheelIndex < 3; ++WheelIndex)
	{
		if (OptionalWheelRefs[WheelIndex].IsNull())
		{
			++OptionalWheelMissingCount;
			continue;
		}
		if (!OptionalWheelSnapshots[WheelIndex]->bAssetLoaded)
		{
			MeshBlockers.Add(FString::Printf(TEXT("Wheel Mesh %s를 resolve하지 못했습니다: %s"), OptionalWheelRoleNames[WheelIndex], *OptionalWheelRefs[WheelIndex].ToSoftObjectPath().ToString()));
			continue;
		}
		if (!HasUsableWheelBounds(*OptionalWheelSnapshots[WheelIndex]))
		{
			MeshBlockers.Add(FString::Printf(TEXT("Wheel Mesh %s bounds가 non-finite이거나 radius/width에 사용할 dimension이 0입니다."), OptionalWheelRoleNames[WheelIndex]));
			continue;
		}
		if (bSocketScaleMode)
		{
			FString CanonicalFailure;
			if (!IsCanonicalSocketScaleWheelMesh(*OptionalWheelSnapshots[WheelIndex], CanonicalFailure))
			{
				MeshBlockers.Add(FString::Printf(TEXT("Socket Scale mode의 Wheel Mesh %s가 canonical 규격을 만족하지 않습니다: %s"), OptionalWheelRoleNames[WheelIndex], *CanonicalFailure));
			}
		}
	}

	if (MeshBlockers.IsEmpty())
	{
		const FString MeshReadySummary = bSocketScaleMode
			? FString::Printf(TEXT("Chassis와 Wheel Mesh가 resolve됐고 Socket Scale mode의 지정 Wheel은 canonical 100x25x100cm + centered bounds를 만족합니다. Optional Wheel 미지정 %d개는 FL runtime fallback을 사용합니다."), OptionalWheelMissingCount)
			: FString::Printf(TEXT("Chassis와 필수 FL Wheel이 fresh AssetSnapshot에서 resolve됐고, 지정된 다른 Wheel도 모두 usable합니다. Optional Wheel 미지정 %d개. +X 전방/Reference scale은 USER 수동 확인 항목입니다."), OptionalWheelMissingCount);
		SetStep(ECFVehicleBuilderStepId::MeshPrep, ECFVehicleBuilderStepState::Complete,
			MeshReadySummary,
			bSocketScaleMode
				? TEXT("Mesh 준비가 완료되었습니다. 타이어 크기는 자동 추정하지 않으며 Step 3에서 USER-authored Socket Scale을 확인합니다.")
				: TEXT("Mesh 준비가 완료되었습니다. 자동 Wheel detection이나 Chassis scale 자동 PASS는 수행하지 않았습니다."),
			true);
	}
	else
	{
		SetStep(ECFVehicleBuilderStepId::MeshPrep, ECFVehicleBuilderStepState::Blocked,
			FString::Printf(TEXT("Mesh 준비 blocker %d개: %s"), MeshBlockers.Num(), *FString::Join(MeshBlockers, TEXT(" / "))),
			TEXT("표시된 Mesh reference를 지정/교정한 뒤 다시 검사하세요. Wheel center 자동검출은 하지 않습니다."),
			true);
	}
}

// 소켓 준비 단계의 current state를 fresh Chassis socket truth에서 평가합니다.
void FCFVehicleBuilderVM::EvaluateSocketGuideStep()
{
	// SocketGuide의 prerequisite인 current MeshPrep projection입니다.
	const FCFVehicleBuilderStepView* MeshPrepStep = FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	if (!MeshPrepStep || MeshPrepStep->State != ECFVehicleBuilderStepState::Complete)
	{
		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Locked,
			TEXT("Socket 검사는 Chassis/필수 Wheel Mesh 준비가 끝난 뒤 진행합니다."),
			TEXT("Step 2 Mesh blocker 또는 stale 상태를 먼저 해결하세요."),
			true);
		return;
	}

	// SocketGuide가 읽을 current persistent Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !bHasCurrentResolveReadForStepDiagnostics)
	{
		return;
	}

	// Current Recipe의 persistent Asset intent입니다.
	const FCFVehicleAssetIntent& AssetIntent = Recipe->AssetIntent;
	// Current fresh asset/socket truth입니다.
	const FCFVehicleAssetSnapshot& Assets = AuthoringViewModel->GetResolveResult().ResolveRequest.Assets;
	// Resolver/AssetReader와 같은 fallback 규칙을 사용한 FL/FR/RL/RR effective socket 이름입니다.
	const TArray<FName> WheelSocketNames = BuildResolvedWheelSocketNames(AssetIntent);
	// Wheel role 중 같은 effective socket 이름을 공유하는지 검사할 집합입니다.
	TSet<FName> UniqueWheelSocketNames;
	for (const FName SocketName : WheelSocketNames)
	{
		UniqueWheelSocketNames.Add(SocketName);
	}
	// 4 role 모두 서로 다른 socket을 가리키는지 여부입니다.
	const bool bWheelSocketBindingsDistinct = UniqueWheelSocketNames.Num() == 4;
	// 실제 Chassis에서 찾은 Wheel role socket 개수입니다.
	int32 FoundWheelSocketCount = 0;
	// custom Wheel socket binding role 개수입니다.
	int32 CustomWheelSocketCount = 0;
	// current snapshot의 4 role socket fact pointer입니다.
	TArray<const FCFVehicleSocketSnapshot*> WheelSocketFacts;
	WheelSocketFacts.Reserve(4);
	// Project 기본 Wheel role 이름입니다.
	const FName DefaultWheelSocketNames[] = {TEXT("Wheel_Anchor_FL"), TEXT("Wheel_Anchor_FR"), TEXT("Wheel_Anchor_RL"), TEXT("Wheel_Anchor_RR")};
	// FL/FR/RL/RR 네 Wheel role을 순회하는 index입니다.
	for (int32 WheelRoleIndex = 0; WheelRoleIndex < 4; ++WheelRoleIndex)
	{
		// current AssetSnapshot에서 exact role 이름으로 찾은 Socket fact입니다.
		const FCFVehicleSocketSnapshot* SocketFact = Assets.FindChassisSocket(WheelSocketNames[WheelRoleIndex]);
		WheelSocketFacts.Add(SocketFact);
		if (SocketFact && SocketFact->bFound)
		{
			++FoundWheelSocketCount;
		}
		if (WheelSocketNames[WheelRoleIndex] != DefaultWheelSocketNames[WheelRoleIndex])
		{
			++CustomWheelSocketCount;
		}
	}

	// SocketScaleFromChassis에서는 네 Wheel Socket의 USER-authored Scale과 axle derived size를 같은 Runtime helper로 검증합니다.
	if (Recipe->WheelVisualIntent.Mode == ECFWheelVisualIntentMode::SocketScaleFromChassis)
	{
		const FCFVehicleWheelAssetSnapshot* EffectiveWheelSnapshots[] =
		{
			&Assets.WheelFL,
			Assets.WheelFR.bAssetLoaded ? &Assets.WheelFR : &Assets.WheelFL,
			Assets.WheelRL.bAssetLoaded ? &Assets.WheelRL : &Assets.WheelFL,
			Assets.WheelRR.bAssetLoaded ? &Assets.WheelRR : &Assets.WheelFL
		};
		FCFDerivedWheelSize DerivedWheelSizes[4];
		TArray<FString> SocketScaleBlockers;
		for (int32 WheelRoleIndex = 0; WheelRoleIndex < 4; ++WheelRoleIndex)
		{
			const FCFVehicleSocketSnapshot* SocketFact = WheelSocketFacts[WheelRoleIndex];
			if (!SocketFact || !SocketFact->bFound)
			{
				continue;
			}
			FString ScaleError;
			if (!FCFWheelSizeUtils::DeriveWheelSizeFromBoundsAndScale(
				EffectiveWheelSnapshots[WheelRoleIndex]->BoundsExtent,
				SocketFact->RelativeScale,
				DerivedWheelSizes[WheelRoleIndex],
				ScaleError))
			{
				SocketScaleBlockers.Add(FString::Printf(TEXT("%s: %s"), *WheelSocketNames[WheelRoleIndex].ToString(), *ScaleError));
			}
		}
		if (SocketScaleBlockers.IsEmpty()
			&& (!FCFWheelSizeUtils::AreWheelSizesCompatible(DerivedWheelSizes[0], DerivedWheelSizes[1])
				|| !FCFWheelSizeUtils::AreWheelSizesCompatible(DerivedWheelSizes[2], DerivedWheelSizes[3])))
		{
			SocketScaleBlockers.Add(TEXT("같은 axle의 좌/우 Socket-derived 타이어 Radius/Width가 일치하지 않습니다."));
		}
		if (!SocketScaleBlockers.IsEmpty())
		{
			SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Blocked,
				FString::Printf(TEXT("Wheel Socket Scale blocker %d개: %s"), SocketScaleBlockers.Num(), *FString::Join(SocketScaleBlockers, TEXT(" / "))),
				TEXT("Static Mesh 에디터에서 USER가 Wheel Socket Scale을 직접 교정하세요. X/Z는 같은 직경 배율, Y는 폭 배율이며 Builder가 자동 보정하지 않습니다."),
				true);
			return;
		}
	}

	// Guided explicit-plan lane에서 Block하는 Hardpoint identity/socket 구조 오류 수입니다.
	int32 HardpointStructuralBlockerCount = 0;
	// Exact SocketName은 유효하지만 current Chassis에 아직 존재하지 않는 Hardpoint 수입니다.
	int32 HardpointMissingSocketCount = 0;
	// Current Chassis에서 exact SocketName을 찾은 Hardpoint 수입니다.
	int32 HardpointFoundSocketCount = 0;
	// HP_ prefix를 따르지 않는 Advanced/custom naming advisory 수입니다.
	int32 HardpointNamingWarningCount = 0;
	// Legacy direct LocalTransform-compatible SocketName None 수입니다.
	int32 HardpointSocketNoneCount = 0;
	// duplicate LocationSlotId를 찾기 위한 집합입니다.
	TSet<FName> HardpointLocationIds;
	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		if (HardpointIntent.LocationSlotId.IsNone() || HardpointLocationIds.Contains(HardpointIntent.LocationSlotId))
		{
			++HardpointStructuralBlockerCount;
		}
		else
		{
			HardpointLocationIds.Add(HardpointIntent.LocationSlotId);
		}

		if (HardpointIntent.SocketName.IsNone())
		{
			++HardpointSocketNoneCount;
			++HardpointStructuralBlockerCount;
			continue;
		}
		if (!HardpointIntent.SocketName.ToString().StartsWith(TEXT("HP_")))
		{
			++HardpointNamingWarningCount;
		}

		const FCFVehicleSocketSnapshot* HardpointSocketFact = Assets.FindChassisSocket(HardpointIntent.SocketName);
		if (HardpointSocketFact && HardpointSocketFact->bFound)
		{
			++HardpointFoundSocketCount;
		}
		else
		{
			++HardpointMissingSocketCount;
		}
	}

	if (!bWheelSocketBindingsDistinct || FoundWheelSocketCount != 4)
	{
		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Blocked,
			FString::Printf(
				TEXT("Wheel Socket 검사: %d/4 발견, distinct binding=%s. FL=%s / FR=%s / RL=%s / RR=%s."),
				FoundWheelSocketCount,
				bWheelSocketBindingsDistinct ? TEXT("PASS") : TEXT("FAIL"),
				*WheelSocketNames[0].ToString(), *WheelSocketNames[1].ToString(), *WheelSocketNames[2].ToString(), *WheelSocketNames[3].ToString()),
			TEXT("누락된 Wheel Socket을 Chassis Static Mesh의 소켓 매니저에서 직접 생성/배치하거나 중복 role binding을 수정한 뒤 다시 검사하세요. Builder는 Socket을 자동 생성·이동하지 않습니다."),
			true);
		return;
	}

	// 4/4 role socket에서 계산한 front axle midpoint입니다.
	const FVector FrontAxleMidpoint = (WheelSocketFacts[0]->RelativeLocation + WheelSocketFacts[1]->RelativeLocation) * 0.5f;
	// 4/4 role socket에서 계산한 rear axle midpoint입니다.
	const FVector RearAxleMidpoint = (WheelSocketFacts[2]->RelativeLocation + WheelSocketFacts[3]->RelativeLocation) * 0.5f;
	// +X 전방 contract 기준 authored wheelbase입니다.
	const float AuthoredWheelbaseCm = FMath::Abs(FrontAxleMidpoint.X - RearAxleMidpoint.X);
	// FL/FR Y 차이로 계산한 front track입니다.
	const float FrontTrackCm = FMath::Abs(WheelSocketFacts[1]->RelativeLocation.Y - WheelSocketFacts[0]->RelativeLocation.Y);
	// RL/RR Y 차이로 계산한 rear track입니다.
	const float RearTrackCm = FMath::Abs(WheelSocketFacts[3]->RelativeLocation.Y - WheelSocketFacts[2]->RelativeLocation.Y);
	// Project +X forward / left=-Y / right=+Y convention이 명백히 뒤집혔는지 여부입니다.
	const bool bRoleTopologySuspicious = FrontAxleMidpoint.X <= RearAxleMidpoint.X
		|| WheelSocketFacts[0]->RelativeLocation.Y >= WheelSocketFacts[1]->RelativeLocation.Y
		|| WheelSocketFacts[2]->RelativeLocation.Y >= WheelSocketFacts[3]->RelativeLocation.Y;
	// Wheel geometry는 모든 Hardpoint Mode에서 공통으로 보여 주는 USER review summary입니다.
	const FString WheelPassSummary = FString::Printf(
		TEXT("Wheel Socket 4/4 + distinct PASS. Wheelbase=%.2fcm, FrontTrack=%.2fcm, RearTrack=%.2fcm. Custom Wheel name=%d, topology review=%s."),
		AuthoredWheelbaseCm, FrontTrackCm, RearTrackCm, CustomWheelSocketCount,
		bRoleTopologySuspicious ? TEXT("필요") : TEXT("정상"));

	switch (Recipe->BuilderHardpointPlanMode)
	{
	case ECFBuilderHardpointPlanMode::LegacyCompatible:
		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Complete,
			FString::Printf(
				TEXT("%s LegacyCompatible Hardpoint=%d, exact Socket found=%d, missing=%d, SocketNone=%d, naming advisory=%d. Existing/custom 구조는 자동 migration하지 않습니다."),
				*WheelPassSummary, Recipe->HardpointIntents.Num(), HardpointFoundSocketCount, HardpointMissingSocketCount, HardpointSocketNoneCount, HardpointNamingWarningCount),
			TEXT("기존 차량 Hardpoint/Mount 보존 계약을 유지합니다. Guided 1:1 계획을 사용하려면 명시적으로 '장착 위치 사용'으로 전환하세요."),
			true);
		return;

	case ECFBuilderHardpointPlanMode::Unspecified:
		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Ready,
			FString::Printf(TEXT("%s Hardpoint 계획 미결정. 현재 Recipe Hardpoint=%d / Mount=%d."), *WheelPassSummary, Recipe->HardpointIntents.Num(), Recipe->MountIntents.Num()),
			TEXT("'장착점 없음' 또는 '장착 위치 사용' 중 하나를 명시적으로 선택하세요. 기존 intent가 있어도 Builder가 silent mode 전환하지 않습니다."),
			true);
		return;

	case ECFBuilderHardpointPlanMode::NoHardpoints:
		if (!Recipe->HardpointIntents.IsEmpty() || !Recipe->MountIntents.IsEmpty())
		{
			SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Blocked,
				FString::Printf(TEXT("%s NoHardpoints와 Recipe semantic data가 충돌합니다. Hardpoint=%d / Mount=%d."), *WheelPassSummary, Recipe->HardpointIntents.Num(), Recipe->MountIntents.Num()),
				TEXT("Mount를 먼저 제거하고 Hardpoint를 제거한 뒤 '장착점 없음'을 다시 확인하세요. 자동 cascade 삭제는 하지 않습니다."),
				true);
			return;
		}
		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Complete,
			FString::Printf(TEXT("%s Hardpoint Plan=NoHardpoints. 장착 위치 0개를 USER가 명시했습니다."), *WheelPassSummary),
			TEXT("장착 위치를 사용하지 않는 차량으로 Step 3을 완료했습니다. 마음이 바뀌면 '장착 위치 사용'으로 전환할 수 있습니다."),
			true);
		return;

	case ECFBuilderHardpointPlanMode::UseHardpoints:
		if (Recipe->HardpointIntents.IsEmpty())
		{
			SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Ready,
				FString::Printf(TEXT("%s Hardpoint Plan=UseHardpoints이지만 아직 장착 위치가 없습니다."), *WheelPassSummary),
				TEXT("위/앞/뒤/좌/우/아래/내부 중 실제 필요한 위치를 하나 이상 추가하세요."),
				true);
			return;
		}
		if (HardpointStructuralBlockerCount > 0)
		{
			SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Blocked,
				FString::Printf(TEXT("%s Hardpoint 구조 blocker=%d. Standard Guided에서는 LocationSlotId unique/non-None + SocketName non-None이 필요합니다."), *WheelPassSummary, HardpointStructuralBlockerCount),
				TEXT("Advanced/custom intent의 중복/빈 identity를 교정하세요. Standard row의 category/identity는 생성 뒤 silent rename하지 않습니다."),
				true);
			return;
		}
		if (HardpointMissingSocketCount > 0)
		{
			SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Ready,
				FString::Printf(TEXT("%s Hardpoint %d개 중 Socket %d개 확인, %d개 아직 없음. naming advisory=%d."), *WheelPassSummary, Recipe->HardpointIntents.Num(), HardpointFoundSocketCount, HardpointMissingSocketCount, HardpointNamingWarningCount),
				TEXT("'Hardpoint Socket 편집하기'로 Chassis StaticMesh를 열어 표시된 exact HP_* Socket을 USER가 직접 생성/배치/저장한 뒤 현재 상태를 다시 확인하세요."),
				true);
			return;
		}

		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Complete,
			FString::Printf(TEXT("%s Hardpoint %d개 exact Socket PASS. naming advisory=%d."), *WheelPassSummary, Recipe->HardpointIntents.Num(), HardpointNamingWarningCount),
			TEXT("Hardpoint 위치 준비가 완료되었습니다. 다음 단계에서도 Builder는 StaticMesh를 자동 수정하거나 저장하지 않습니다."),
			true);
		return;
	default:
		SetStep(ECFVehicleBuilderStepId::SocketGuide, ECFVehicleBuilderStepState::Blocked,
			TEXT("지원하지 않는 Builder Hardpoint Plan Mode입니다."),
			TEXT("Recipe Hardpoint Plan metadata를 확인하세요."),
			true);
		return;
	}
}

// Layout Capture 단계의 current persisted/current socket equality를 평가합니다.
void FCFVehicleBuilderVM::EvaluateLayoutCaptureStep()
{
	// LayoutCapture의 prerequisite인 current SocketGuide projection입니다.
	const FCFVehicleBuilderStepView* SocketGuideStep = FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	if (!SocketGuideStep || SocketGuideStep->State != ECFVehicleBuilderStepState::Complete)
	{
		// SocketGuide 결과를 보존해 Layout을 Blocked 또는 Locked로 파생한 state입니다.
		const ECFVehicleBuilderStepState LayoutState =
			SocketGuideStep && SocketGuideStep->State == ECFVehicleBuilderStepState::Blocked
				? ECFVehicleBuilderStepState::Blocked
				: ECFVehicleBuilderStepState::Locked;
		SetStep(ECFVehicleBuilderStepId::LayoutCapture, LayoutState,
			TEXT("현재 Chassis/Socket truth로는 Layout Capture prerequisite가 충족되지 않았습니다."),
			TEXT("Wheel role Socket 4/4 존재와 distinct binding을 먼저 충족하세요. 자동 Capture/Save는 수행하지 않습니다."),
			true);
		return;
	}

	// LayoutCapture가 읽을 current persistent Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !bHasCurrentResolveReadForStepDiagnostics)
	{
		return;
	}

	// Current Recipe의 persistent Asset intent입니다.
	const FCFVehicleAssetIntent& AssetIntent = Recipe->AssetIntent;
	// Current fresh asset/socket truth입니다.
	const FCFVehicleAssetSnapshot& Assets = AuthoringViewModel->GetResolveResult().ResolveRequest.Assets;
	// FL/FR/RL/RR effective socket names입니다.
	const TArray<FName> WheelSocketNames = BuildResolvedWheelSocketNames(AssetIntent);
	// Current fresh 4-role socket facts입니다.
	TArray<const FCFVehicleSocketSnapshot*> WheelSocketFacts;
	WheelSocketFacts.Reserve(4);
	// 네 Wheel role의 effective Socket 이름을 순회합니다.
	for (const FName WheelSocketName : WheelSocketNames)
	{
		WheelSocketFacts.Add(Assets.FindChassisSocket(WheelSocketName));
	}
	// current 4-role Socket fact가 모두 실제 존재하는지 순회 확인합니다.
	for (const FCFVehicleSocketSnapshot* SocketFact : WheelSocketFacts)
	{
		if (!SocketFact || !SocketFact->bFound)
		{
			SetStep(ECFVehicleBuilderStepId::LayoutCapture, ECFVehicleBuilderStepState::Blocked,
				TEXT("SocketGuide 이후 fresh snapshot에서 Wheel Socket fact가 사라졌습니다."),
				TEXT("현재 상태를 다시 확인하고 Wheel Socket 4/4를 복구하세요."),
				true);
			return;
		}
	}

	// Current persisted capture truth owner입니다.
	const UCFVehicleData* TargetVehicleData = AuthoringViewModel->GetTargetVehicleData();
	if (!TargetVehicleData)
	{
		SetStep(ECFVehicleBuilderStepId::LayoutCapture, ECFVehicleBuilderStepState::Blocked,
			TEXT("현재 Target VehicleData readback을 찾을 수 없습니다."),
			TEXT("대상을 다시 선택해 Target/Recipe binding을 fresh read하세요."),
			true);
		return;
	}

	if (!TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides)
	{
		SetStep(ECFVehicleBuilderStepId::LayoutCapture, ECFVehicleBuilderStepState::Ready,
			FString::Printf(TEXT("Layout Capture 준비 완료입니다. current ChassisLayoutFingerprint=%s. VehicleLayoutConfig.bUseLayoutOverrides=false이므로 아직 Target에는 NotCaptured 상태이며 실제 Layout Apply는 Step 7에서 수행합니다."), *Assets.ChassisLayoutFingerprint),
			TEXT("현재 Socket truth는 Step 5 이후 proposal에 사용할 수 있습니다. 이 Ready/NotCaptured 상태에서는 다음 단계 진행을 허용하며, 실제 Target Layout capture/apply는 Step 7 Final Review에서 explicit USER 승인으로 수행합니다."),
			true);
		return;
	}

	// Resolver precedence와 무관하게 비교할 current persisted VehicleLayoutConfig입니다.
	const FCFVehicleLayoutConfig& PersistedLayout = TargetVehicleData->VehicleLayoutConfig;
	// persisted FL/FR/RL/RR Socket binding입니다.
	const FName PersistedWheelSocketNames[] =
	{
		PersistedLayout.BodyWheelSocketFL,
		PersistedLayout.BodyWheelSocketFR,
		PersistedLayout.BodyWheelSocketRL,
		PersistedLayout.BodyWheelSocketRR
	};
	// persisted FL/FR/RL/RR Wheel Anchor pose입니다.
	const FCFWheelAnchorPose* PersistedWheelAnchorPoses[] =
	{
		&PersistedLayout.WheelAnchorFL,
		&PersistedLayout.WheelAnchorFR,
		&PersistedLayout.WheelAnchorRL,
		&PersistedLayout.WheelAnchorRR
	};
	// Current AssetSnapshot과 persisted Target의 direct exact mismatch 수입니다.
	int32 WheelLayoutMismatchCount = 0;
	// persisted/current 비교를 위해 FL/FR/RL/RR role을 순회하는 index입니다.
	for (int32 WheelRoleIndex = 0; WheelRoleIndex < 4; ++WheelRoleIndex)
	{
		if (PersistedWheelSocketNames[WheelRoleIndex] != WheelSocketNames[WheelRoleIndex])
		{
			++WheelLayoutMismatchCount;
		}
		if (!AreVectorsExactlyEqual(PersistedWheelAnchorPoses[WheelRoleIndex]->RelativeLocation, WheelSocketFacts[WheelRoleIndex]->RelativeLocation)
			|| !AreRotatorsExactlyEqual(PersistedWheelAnchorPoses[WheelRoleIndex]->RelativeRotation, WheelSocketFacts[WheelRoleIndex]->RelativeRotation)
			|| !AreVectorsExactlyEqual(PersistedWheelAnchorPoses[WheelRoleIndex]->RelativeScale, WheelSocketFacts[WheelRoleIndex]->RelativeScale))
		{
			++WheelLayoutMismatchCount;
		}
	}

	// Socket-bound Hardpoint persisted capture mismatch는 P0-03 Warning으로만 집계합니다.
	int32 HardpointLayoutWarningCount = 0;
	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		if (HardpointIntent.LocationSlotId.IsNone() || HardpointIntent.SocketName.IsNone())
		{
			continue;
		}

		// Current AssetSnapshot에서 이 Hardpoint intent가 요청한 Socket fact입니다.
		const FCFVehicleSocketSnapshot* HardpointSocketFact = Assets.FindChassisSocket(HardpointIntent.SocketName);
		// Current Target에서 같은 stable LocationSlotId로 materialized된 persisted Hardpoint slot입니다.
		const FCFVehicleHardpointSlot* PersistedHardpoint = TargetVehicleData->HardpointSlots.FindByPredicate([&HardpointIntent](const FCFVehicleHardpointSlot& HardpointSlot)
		{
			return HardpointSlot.LocationSlotId == HardpointIntent.LocationSlotId;
		});
		if (!HardpointSocketFact || !HardpointSocketFact->bFound || !PersistedHardpoint
			|| !AreVectorsExactlyEqual(PersistedHardpoint->LocalLocation, HardpointSocketFact->RelativeLocation)
			|| !AreRotatorsExactlyEqual(PersistedHardpoint->LocalRotation, HardpointSocketFact->RelativeRotation))
		{
			++HardpointLayoutWarningCount;
		}
	}

	if (WheelLayoutMismatchCount == 0)
	{
		SetStep(ECFVehicleBuilderStepId::LayoutCapture, ECFVehicleBuilderStepState::Complete,
			FString::Printf(
				TEXT("Persisted VehicleLayoutConfig가 current AssetSnapshot의 Wheel Socket identity/location/rotation/scale과 direct exact 일치합니다. ChassisLayoutFingerprint=%s. Hardpoint layout warning=%d."),
				*Assets.ChassisLayoutFingerprint, HardpointLayoutWarningCount),
			TEXT("Wheel Layout은 Current입니다. Hardpoint mismatch가 있더라도 P0-03에서는 Warning이며 requiredness는 Gameplay Setup이 결정합니다."),
			true);
	}
	else
	{
		SetStep(ECFVehicleBuilderStepId::LayoutCapture, ECFVehicleBuilderStepState::Stale,
			FString::Printf(
				TEXT("기존 persisted Layout과 current Socket truth가 다릅니다. Wheel layout mismatch=%d, Hardpoint layout warning=%d, ChassisLayoutFingerprint=%s."),
				WheelLayoutMismatchCount, HardpointLayoutWarningCount, *Assets.ChassisLayoutFingerprint),
			TEXT("현재 Chassis Socket 위치/이름/Scale을 검토한 뒤 reviewed Capture/Apply 경로로 다시 반영하세요. 이 evaluator는 자동 Capture·Socket 이동·Save를 하지 않습니다."),
			true);
	}
}

// Physics Proposal 단계의 accepted Evidence/private 4 Profile/receipt current truth를 fresh 평가합니다.
void FCFVehicleBuilderVM::EvaluatePhysicsProposalStep()
{
	// Step 1~4 prerequisite 상태 diagnostic입니다.
	FString PrerequisiteError;
	if (!ArePhysicsProposalPrerequisitesComplete(PrerequisiteError))
	{
		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Locked,
			PrerequisiteError,
			TEXT("차량/Reference, Mesh, Socket, Layout을 먼저 Current Complete 상태로 만들어야 합니다."),
			true);
		return;
	}

	// Current exact Builder-private Profile missing 개수입니다.
	int32 MissingProfileCount = 0;
	// Current Profile owner/binding diagnostic입니다.
	FString ProfileError;
	if (!ReadPrivateProfileCompleteness(MissingProfileCount, ProfileError))
	{
		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Blocked,
			ProfileError,
			TEXT("shared/foreign/invalid Profile을 덮어쓰지 않습니다. current Recipe의 Builder-private exact owner binding을 먼저 복구하세요."),
			true);
		return;
	}
	if (MissingProfileCount > 0)
	{
		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Blocked,
			FString::Printf(TEXT("Physics Proposal에 필요한 Builder-private Profile이 %d개 누락됐습니다."), MissingProfileCount),
			TEXT("Step 1 Companion flow에서 Missing private Profile을 complete seed와 함께 생성한 뒤 다시 확인하세요."),
			true);
		return;
	}

	// Current exact Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	// Current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !Evidence || !ReferenceEvidenceStateError.IsEmpty())
	{
		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Blocked,
			ReferenceEvidenceStateError.IsEmpty()
				? TEXT("Physics Proposal에 binding할 current Reference Evidence가 없습니다.")
				: ReferenceEvidenceStateError,
			TEXT("Step 1에서 current Reference Evidence binding/fingerprint를 먼저 정상화하세요."),
			true);
		return;
	}

	// Step 1 local token 또는 matching persistent receipt가 current exact Reference identity/fingerprint를 증명하는지 여부입니다.
	const bool bReferenceAccepted = IsCurrentReferenceAcceptedForProgress();
	if (!bReferenceAccepted)
	{
		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Locked,
			TEXT("Current Reference Evidence가 USER-reviewed exact fingerprint 상태가 아닙니다."),
			TEXT("Step 1에서 현재 Reference Set을 다시 확인한 뒤 Physics Proposal을 진행하세요."),
			true);
		return;
	}

	if (bHasLoadedPhysicsProposalDraft)
	{
		// Loaded AI Draft를 current exact authority에 binding한 typed request입니다.
		FCFBuilderProfileCommitRequest DraftRequest;
		// Draft binding 또는 typed preview diagnostic입니다.
		FString DraftError;
		if (!BuildPhysicsProposalRequest(DraftRequest, DraftError))
		{
			SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Stale,
				DraftError,
				TEXT("구버전/다른 Evidence Proposal을 적용하지 않습니다. AI PhysicsDraft를 current Reference/Profile truth에서 다시 생성하세요."),
				true);
			return;
		}

		DraftRequest.CallContext.ClientOperationId = TEXT("VB-P0-09-Physics-Evaluate-Draft");
		// Loaded proposal의 current mutation0 typed preview입니다.
		FCFBuilderProfileCommitPreview DraftPreview;
		if (!FCFVehicleAuthoringService::PreviewBuilderProfiles(DraftRequest, DraftPreview))
		{
			SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Stale,
				DraftPreview.Operation.Message,
				TEXT("Evidence/Profile/Recipe/Target가 draft 이후 달라졌거나 typed validation이 막혔습니다. AI Proposal을 fresh하게 다시 준비하세요."),
				true);
			return;
		}

		if (DraftPreview.Operation.Status == ECFAuthoringOpStatus::NoChange)
		{
			SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Complete,
				TEXT("Loaded AI Physics Proposal과 current private 4 Profile, Evidence, persistent Builder receipt가 exact 일치합니다."),
				TEXT("Physics Proposal은 Current입니다. 다음 Gameplay Setup으로 진행할 수 있습니다."),
				true);
			return;
		}

		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Ready,
			FString::Printf(
				TEXT("AI Physics Proposal이 current Reference Evidence에 binding되어 mutation0 typed preview를 통과할 수 있습니다. %s"),
				*LoadedPhysicsProposalDraft.UserFacingSummary),
			TEXT("'Physics Proposal 검토 후 반영'에서 변경 범위와 근거를 확인한 뒤 명시적으로 승인하세요. Target VehicleData Apply와 Save는 수행하지 않습니다."),
			true);
		return;
	}

	if (Recipe->BuilderCommitReceipt.IsValid())
	{
		// Persistent receipt가 current truth를 여전히 증명하는지 확인할 current-payload read-only request입니다.
		FCFBuilderProfileCommitRequest ReceiptRequest;
		// Receipt request build diagnostic입니다.
		FString ReceiptError;
		if (!BuildCurrentPhysicsReceiptRequest(ReceiptRequest, ReceiptError))
		{
			SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Stale,
				ReceiptError,
				TEXT("Current Reference/private Profile truth에서 AI Physics Proposal을 다시 생성해 review/commit하세요."),
				true);
			return;
		}

		ReceiptRequest.CallContext.ClientOperationId = TEXT("VB-P0-09-Physics-Evaluate-Receipt");
		// Existing facade가 evidence/claim/profile/resolver/receipt를 fresh 비교한 mutation0 result입니다.
		FCFBuilderProfileCommitPreview ReceiptPreview;
		if (!FCFVehicleAuthoringService::PreviewBuilderProfiles(ReceiptRequest, ReceiptPreview))
		{
			SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Stale,
				ReceiptPreview.Operation.Message,
				TEXT("Persistent Builder receipt를 current truth에 재사용할 수 없습니다. AI Physics Proposal을 fresh하게 다시 review/commit하세요."),
				true);
			return;
		}

		if (ReceiptPreview.Operation.Status == ECFAuthoringOpStatus::NoChange)
		{
			SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Complete,
				FString::Printf(
					TEXT("Persistent Builder receipt가 current Evidence/Claim set/private 4 Profile/Resolver를 exact 증명합니다. EvidenceFingerprint=%s."),
					*ReceiptPreview.EvidenceFingerprint),
				TEXT("Physics Proposal은 Current입니다. 다음 Gameplay Setup으로 진행할 수 있습니다."),
				true);
			return;
		}

		SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Stale,
			ReceiptPreview.Operation.Message,
			TEXT("Current private Profile payload와 receipt가 동일한 accepted proposal을 증명하지 않습니다. fresh AI Physics Proposal을 다시 review/commit하세요."),
			true);
		return;
	}

	SetStep(ECFVehicleBuilderStepId::PhysicsProposal, ECFVehicleBuilderStepState::Ready,
		TEXT("Step 1~4와 private 4 Profile은 준비됐지만 accepted Physics Proposal receipt가 아직 없습니다."),
		FString::Printf(
			TEXT("AI가 current Reference Evidence를 근거로 %s 에 complete private 4 Profile Proposal을 작성한 뒤 불러오고 검토하세요."),
			*GetPhysicsProposalDraftPath()),
		true);
}

// Step 6이 요구하는 Step 5 current Complete prerequisite를 검사합니다.
bool FCFVehicleBuilderVM::AreGameplaySetupPrerequisitesComplete(FString& OutError) const
{
	// Step 6의 immediate prerequisite인 Physics Proposal projection입니다.
	const FCFVehicleBuilderStepView* PhysicsStep = FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	if (!PhysicsStep || PhysicsStep->State != ECFVehicleBuilderStepState::Complete)
	{
		OutError = PhysicsStep
			? FString::Printf(
				TEXT("Gameplay Setup 전에 Physics Proposal이 Complete여야 합니다. 현재 상태: %s"),
				StepStateText(PhysicsStep->State))
			: TEXT("Gameplay Setup prerequisite Physics Proposal Step을 찾을 수 없습니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// Current managed Recipe/Target을 existing Gameplay Guidance R0 request로 구성합니다.
bool FCFVehicleBuilderVM::BuildGameplayGuidanceRequest(
	FCFBuilderGameplayGuidanceRequest& OutRequest,
	FString& OutError) const
{
	OutRequest = FCFBuilderGameplayGuidanceRequest();

	// Gameplay Guidance semantic owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || IsMeshOnlyCandidate() || !Recipe->RecipeId.IsValid())
	{
		OutError = TEXT("Gameplay Setup을 읽을 valid managed Recipe가 없습니다.");
		return false;
	}

	// Gameplay Guidance가 current baseline/diff를 비교할 Target VehicleData입니다.
	UCFVehicleData* TargetVehicleData = Recipe->TargetVehicleData.Get();
	if (!TargetVehicleData)
	{
		TargetVehicleData = Recipe->TargetVehicleData.LoadSynchronous();
	}
	if (!TargetVehicleData)
	{
		OutError = TEXT("Gameplay Setup을 읽을 current Target VehicleData를 load할 수 없습니다.");
		return false;
	}

	OutRequest.ReadRequest.Recipe = Recipe;
	OutRequest.ReadRequest.TargetVehicleData = TargetVehicleData;
	OutRequest.ReadRequest.CallerKind = ECFAuthoringCallerKind::SlateUI;
	OutRequest.Mode = DeriveCompanionMode();
	OutRequest.HardpointPlanMode = Recipe->BuilderHardpointPlanMode;
	OutError.Reset();
	return true;
}

// USER-facing Step 6 8영역 completeness/manual Socket/pending diff 요약을 만듭니다.
FString FCFVehicleBuilderVM::BuildGameplayGuidanceSummary() const
{
	if (!bHasGameplayGuidanceResult)
	{
		return TEXT("Gameplay Guidance를 아직 fresh read하지 못했습니다. '현재 상태 다시 확인'으로 authoritative truth를 다시 읽으세요.");
	}

	// 현재 Gameplay Guidance aggregate summary입니다.
	FString Summary = FString::Printf(
		TEXT("모드: %s\n영역: %d개 | 확인 필요: %d | 막힘: %d | Step 완료 가능: %s\nGameplay pending diff: %d\n%s"),
		GameplayGuidanceResult.Mode == ECFBuilderCompanionMode::CompleteExisting ? TEXT("Existing Vehicle Completion") : TEXT("New Vehicle"),
		GameplayGuidanceResult.Items.Num(),
		GameplayGuidanceResult.NeedsReviewCount,
		GameplayGuidanceResult.BlockedCount,
		GameplayGuidanceResult.bCanCompleteGameplayStep ? TEXT("예") : TEXT("아니오"),
		GameplayGuidanceResult.PendingGameplayDiffCount,
		*GameplayGuidanceResult.ExistingCompletionSummary);

	for (const FCFBuilderGameplayGuidanceItem& Item : GameplayGuidanceResult.Items)
	{
		Summary += FString::Printf(
			TEXT("\n\n[%s] %s\n%s"),
			GameplayAreaText(Item.Area),
			GameplayGuidanceStateText(Item.State),
			*Item.Summary);

		if (!Item.ResolutionText.IsEmpty()
			&& (Item.State == ECFBuilderGuidanceState::NeedsReview || Item.State == ECFBuilderGuidanceState::Blocked))
		{
			Summary += FString::Printf(TEXT("\n→ %s"), *Item.ResolutionText);
		}

		if (!Item.RelatedFieldPath.IsEmpty())
		{
			Summary += FString::Printf(TEXT("\nField: %s"), *Item.RelatedFieldPath);
		}
	}

	if (!GameplayGuidanceResult.SocketGuidance.IsEmpty())
	{
		Summary += TEXT("\n\nUSER Socket 안내");
		for (const FCFBuilderManualSocketGuidance& SocketGuidance : GameplayGuidanceResult.SocketGuidance)
		{
			Summary += FString::Printf(
				TEXT("\n- %s / %s → %s | Chassis: %s | Existing transform 보존: %s\n  %s"),
				GameplayAreaText(SocketGuidance.Area),
				*SocketGuidance.SemanticId.ToString(),
				*SocketGuidance.SocketName.ToString(),
				SocketGuidance.bFoundOnChassis ? TEXT("있음") : TEXT("없음"),
				SocketGuidance.bExistingStoredTransformAccepted ? TEXT("예") : TEXT("아니오"),
				*SocketGuidance.Instruction);
		}
	}

	Summary += TEXT("\n\n8영역 Gameplay Guidance와 Socket 진단은 read-only입니다. Standard Mount 패널의 명시적 반영/삭제만 Recipe MountIntents를 변경합니다. Step 3의 '추가 후 편집'은 USER가 선택한 exact SocketName을 Chassis 원점에 명시적으로 생성할 수 있지만 자동 배치/자동 저장은 하지 않습니다. Target VehicleData Apply는 수행하지 않습니다.");
	if (GameplayGuidanceResult.PendingGameplayDiffCount > 0)
	{
		Summary += TEXT("\nGameplay pending diff는 Step 7 Final Review에서 explicit Apply 여부를 검토합니다.");
	}
	return Summary;
}

// Gameplay Setup 단계의 existing R0 Gameplay Guidance authority를 fresh 평가합니다.
void FCFVehicleBuilderVM::EvaluateGameplaySetupStep()
{
	GameplayGuidanceResult = FCFBuilderGameplayGuidanceResult();
	bHasGameplayGuidanceResult = false;

	// Step 6은 accepted/current Physics Proposal 위에서만 Gameplay completeness를 평가합니다.
	FString PrerequisiteError;
	if (!AreGameplaySetupPrerequisitesComplete(PrerequisiteError))
	{
		SetStep(
			ECFVehicleBuilderStepId::GameplaySetup,
			ECFVehicleBuilderStepState::Locked,
			PrerequisiteError,
			TEXT("Step 5 Physics Proposal을 current Complete 상태로 만든 뒤 다시 확인하세요."),
			true);
		return;
	}

	// Existing ReadBuilderGameplayGuidance facade에 넘길 current Recipe/Target request입니다.
	FCFBuilderGameplayGuidanceRequest Request;
	// Request build diagnostic입니다.
	FString RequestError;
	if (!BuildGameplayGuidanceRequest(Request, RequestError))
	{
		SetStep(
			ECFVehicleBuilderStepId::GameplaySetup,
			ECFVehicleBuilderStepState::Blocked,
			RequestError,
			TEXT("Current Recipe/Target binding을 복구한 뒤 다시 확인하세요. Builder가 누락 값을 추정하거나 raw write하지 않습니다."),
			true);
		return;
	}

	// Existing R0 Gameplay completeness/read-only guidance 결과입니다.
	FCFBuilderGameplayGuidanceResult GuidanceResult;
	if (!FCFVehicleAuthoringService::ReadBuilderGameplayGuidance(Request, GuidanceResult))
	{
		GameplayGuidanceResult = GuidanceResult;
		bHasGameplayGuidanceResult = true;
		SetStep(
			ECFVehicleBuilderStepId::GameplaySetup,
			ECFVehicleBuilderStepState::Blocked,
			GuidanceResult.Operation.Message.IsEmpty()
				? TEXT("Gameplay Guidance R0 read가 current authoritative truth를 평가하지 못했습니다.")
				: GuidanceResult.Operation.Message,
			TEXT("오류를 확인하고 current Recipe/Profile/Target/Asset binding을 복구한 뒤 fresh read하세요."),
			true);
		return;
	}

	GameplayGuidanceResult = MoveTemp(GuidanceResult);
	bHasGameplayGuidanceResult = true;

	if (GameplayGuidanceResult.BlockedCount > 0)
	{
		SetStep(
			ECFVehicleBuilderStepId::GameplaySetup,
			ECFVehicleBuilderStepState::Blocked,
			FString::Printf(
				TEXT("Gameplay Setup 8영역 중 %d개가 current authoring contract 위반으로 막혀 있습니다. 확인 필요 %d개."),
				GameplayGuidanceResult.BlockedCount,
				GameplayGuidanceResult.NeedsReviewCount),
			TEXT("Step 6 상세 안내에서 막힌 영역의 Related Field/해결법을 확인하세요. 자동 보충·자동 Asset 생성·자동 Socket 배치는 하지 않습니다."),
			true);
		return;
	}

	if (GameplayGuidanceResult.NeedsReviewCount > 0)
	{
		SetStep(
			ECFVehicleBuilderStepId::GameplaySetup,
			ECFVehicleBuilderStepState::Ready,
			FString::Printf(
				TEXT("Gameplay Guidance R0 기준 %d개 영역에 USER 확인/수동 작업이 남아 있습니다. Standard Mount explicit Recipe write와 별개이며 Pending Gameplay Diff=%d."),
				GameplayGuidanceResult.NeedsReviewCount,
				GameplayGuidanceResult.PendingGameplayDiffCount),
			TEXT("상세 안내의 Socket/field를 확인해 USER가 필요한 작업만 수행한 뒤 '현재 상태 다시 확인'을 누르세요. Socket 위치는 USER authority입니다."),
			true);
		return;
	}

	if (GameplayGuidanceResult.bCanCompleteGameplayStep)
	{
		SetStep(
			ECFVehicleBuilderStepId::GameplaySetup,
			ECFVehicleBuilderStepState::Complete,
			FString::Printf(
				TEXT("Gameplay Setup 8영역이 current authoritative truth에서 Complete/Optional입니다. Pending Gameplay Diff=%d."),
				GameplayGuidanceResult.PendingGameplayDiffCount),
			GameplayGuidanceResult.PendingGameplayDiffCount > 0
				? TEXT("Gameplay pending diff는 정상입니다. 이 단계는 적용하지 않으며 Step 7 Final Review에서 explicit Apply를 검토하세요.")
				: TEXT("추가 Gameplay 수동 작업이 없습니다. Step 7 Final Review로 진행할 수 있습니다."),
			true);
		return;
	}

	SetStep(
		ECFVehicleBuilderStepId::GameplaySetup,
		ECFVehicleBuilderStepState::Blocked,
		TEXT("Gameplay Guidance가 blocker/review 0인데도 completion contract를 만족하지 못했습니다."),
		TEXT("현재 Guidance result를 확인하고 backend contract mismatch로 취급하세요. 자동 우회하지 않습니다."),
		true);
}

// Step 7이 요구하는 Step 6 current Complete prerequisite를 검사합니다.
bool FCFVehicleBuilderVM::AreFinalReviewPrerequisitesComplete(FString& OutError) const
{
	// Step 7의 immediate prerequisite인 Gameplay Setup projection입니다.
	const FCFVehicleBuilderStepView* GameplayStep = FindStepView(ECFVehicleBuilderStepId::GameplaySetup);
	if (!GameplayStep || GameplayStep->State != ECFVehicleBuilderStepState::Complete)
	{
		OutError = GameplayStep
			? FString::Printf(
				TEXT("Final Review 전에 Gameplay Setup이 Complete여야 합니다. 현재 상태: %s"),
				StepStateText(GameplayStep->State))
			: TEXT("Final Review prerequisite Gameplay Setup Step을 찾을 수 없습니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// Current managed Recipe/Target/Evidence/receipt를 existing Final Review R0 request로 구성합니다.
bool FCFVehicleBuilderVM::BuildFinalReviewRequest(
	FCFBuilderFinalReviewRequest& OutRequest,
	FString& OutError) const
{
	OutRequest = FCFBuilderFinalReviewRequest();

	// Final Review가 재사용할 current Gameplay Guidance read request입니다.
	FCFBuilderGameplayGuidanceRequest GameplayRequest;
	if (!BuildGameplayGuidanceRequest(GameplayRequest, OutError))
	{
		return false;
	}

	// Persistent provenance binding의 current Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = CurrentReferenceEvidence.Get();
	if (!Evidence || !CurrentReferenceEvidencePath.IsValid() || !Evidence->EvidenceId.IsValid() || Evidence->EvidenceFingerprint.IsEmpty())
	{
		OutError = TEXT("Final Review에 사용할 current Reference Evidence identity/fingerprint가 없습니다.");
		return false;
	}

	OutRequest.GameplayRequest = GameplayRequest;
	OutRequest.bHasEvidenceBinding = true;
	OutRequest.EvidenceBinding.EvidencePath = CurrentReferenceEvidencePath;
	OutRequest.EvidenceBinding.ExpectedEvidenceId = Evidence->EvidenceId;
	OutRequest.EvidenceBinding.ExpectedEvidenceFingerprint = Evidence->EvidenceFingerprint;
	// Consumed Claim canonical set은 persistent BuilderCommitReceipt가 소유하므로 resume-safe backend fallback을 사용합니다.
	OutRequest.EvidenceBinding.ConsumedClaimIds.Reset();
	OutError.Reset();
	return true;
}

// USER-facing Step 7 validation/drift/provenance/diff/apply readiness 요약을 만듭니다.
FString FCFVehicleBuilderVM::BuildFinalReviewSummary() const
{
	if (!bHasFinalReviewResult)
	{
		return TEXT("Final Review를 아직 fresh read하지 못했습니다. '현재 상태 다시 확인'으로 authoritative truth를 다시 읽으세요.");
	}

	// Current Final Review aggregate summary입니다.
	FString Summary = FString::Printf(
		TEXT(
			"Warning: %d | Blocker: %d | External Drift: %s\n"
			"Target Diff: %d | Apply 필요: %s | Apply 가능: %s | Final Review 완료 가능: %s\n"
			"Guarded Undo token: %s\n"
			"DiffHash: %s"),
		FinalReviewResult.WarningCount,
		FinalReviewResult.BlockingIssueCount,
		FinalReviewResult.bHasExternalDrift ? TEXT("있음") : TEXT("없음"),
		FinalReviewResult.FieldDiff.Num(),
		FinalReviewResult.bApplyRequired ? TEXT("예") : TEXT("아니오"),
		FinalReviewResult.bCanApply ? TEXT("예") : TEXT("아니오"),
		FinalReviewResult.bCanCompleteFinalReview ? TEXT("예") : TEXT("아니오"),
		bHasFinalReviewUndoToken ? TEXT("있음") : TEXT("없음"),
		FinalReviewResult.DiffHash.IsEmpty() ? TEXT("<없음>") : *FinalReviewResult.DiffHash);

	if (FinalReviewResult.Provenance.bAvailable)
	{
		Summary += FString::Printf(
			TEXT(
				"\n\nReference provenance: PASS"
				"\n- Consumed Claim: %d"
				"\n- FACT: %d"
				"\n- DERIVED: %d"
				"\n- GAME_BIAS: %d"
				"\n- EvidenceFingerprint: %s"),
			FinalReviewResult.Provenance.ConsumedClaimCount,
			FinalReviewResult.Provenance.FactClaimCount,
			FinalReviewResult.Provenance.DerivedClaimCount,
			FinalReviewResult.Provenance.GameBiasClaimCount,
			*FinalReviewResult.Provenance.EvidenceFingerprint);
	}
	else
	{
		Summary += FString::Printf(
			TEXT("\n\nReference provenance: BLOCKED\n%s"),
			FinalReviewResult.Provenance.IssueText.IsEmpty()
				? TEXT("<provenance diagnostic 없음>")
				: *FinalReviewResult.Provenance.IssueText);
	}

	if (FinalReviewResult.TransmissionDiagnostic.bEvaluated)
	{
		Summary += FString::Printf(
			TEXT("\n\nTransmission Review: %s\n- ProposalHash: %s"),
			FinalReviewResult.TransmissionDiagnostic.bVehicleSpecificRequired ? TEXT("VehicleSpecificRequired") : TEXT("LegacyCompatible"),
			FinalReviewResult.TransmissionDiagnostic.TransmissionProposalHash.IsEmpty()
				? TEXT("<Legacy/미검토>")
				: *FinalReviewResult.TransmissionDiagnostic.TransmissionProposalHash);

		for (const FCFBuilderTransmissionGearDiagnostic& GearDiagnostic : FinalReviewResult.TransmissionDiagnostic.Gears)
		{
			// USER가 고정 ChangeUpRPM 조합을 이해할 수 있게 표시할 한 gear diagnostic row입니다.
			const FString ShiftSpeedText = GearDiagnostic.bShiftSpeedAvailable
				? (FMath::IsNearlyEqual(GearDiagnostic.ShiftSpeedMinKmh, GearDiagnostic.ShiftSpeedMaxKmh, 0.01f)
					? FString::Printf(TEXT("%.1f km/h"), GearDiagnostic.ShiftSpeedMinKmh)
					: FString::Printf(TEXT("%.1f~%.1f km/h"), GearDiagnostic.ShiftSpeedMinKmh, GearDiagnostic.ShiftSpeedMaxKmh))
				: TEXT("Unavailable");
			// 다음 기어가 없는 top gear의 post-shift 표현입니다.
			const FString PostShiftText = GearDiagnostic.bPostShiftAvailable
				? FString::Printf(TEXT("%.0f RPM (Retention %.3f / Down margin %.0f)"), GearDiagnostic.PostShiftRPM, GearDiagnostic.RpmRetention, GearDiagnostic.DownshiftMarginRPM)
				: TEXT("N/A");
			Summary += FString::Printf(
				TEXT("\n- %d단: Ratio %.4f | Overall %.4f | Shift@UpRPM %s | 변속 후 %s"),
				GearDiagnostic.GearNumber,
				GearDiagnostic.GearRatio,
				GearDiagnostic.OverallRatio,
				*ShiftSpeedText,
				*PostShiftText);
		}
		for (const FString& TransmissionWarning : FinalReviewResult.TransmissionDiagnostic.Warnings)
		{
			Summary += FString::Printf(TEXT("\n- Warning: %s"), *TransmissionWarning);
		}
		for (const FString& TransmissionBlocker : FinalReviewResult.TransmissionDiagnostic.Blockers)
		{
			Summary += FString::Printf(TEXT("\n- Blocker: %s"), *TransmissionBlocker);
		}
		Summary += TEXT("\n- 예상 차속은 runtime 변속 조건이 아니라 무슬립 기구학 sanity diagnostic입니다.");
	}

	if (FinalReviewResult.bCanApply)
	{
		Summary += FString::Printf(
			TEXT(
				"\n\nDefinitionApply Proposal"
				"\n- ProposalHash: %s"
				"\n- ResolvedDefinitionHash: %s"
				"\n- ResolverRevision: %d"
				"\n- Target mutation: %s"
				"\n- Auto Save: %s"),
			*FinalReviewResult.ApplyProposal.ProposalHash,
			*FinalReviewResult.ApplyProposal.ProspectiveResolvedDefinitionHash,
			FinalReviewResult.ApplyProposal.ResolverContractRevision,
			FinalReviewResult.ApplyProposal.bTargetMutation ? TEXT("예") : TEXT("아니오"),
			FinalReviewResult.ApplyProposal.bSavePerformed ? TEXT("예") : TEXT("아니오"));
	}

	if (!FinalReviewResult.FieldDiff.IsEmpty())
	{
		Summary += TEXT("\n\n적용 예정 Field Diff");
		for (int32 DiffIndex = 0; DiffIndex < FinalReviewResult.FieldDiff.Num(); ++DiffIndex)
		{
			// USER가 검토할 exact resolver field diff row입니다.
			const FCFVehicleFieldDiff& Diff = FinalReviewResult.FieldDiff[DiffIndex];
			Summary += FString::Printf(
				TEXT("\n\n%d. %s\n   현재: %s\n   적용 후: %s"),
				DiffIndex + 1,
				*Diff.FieldPath.ToCanonicalString(true),
				Diff.bHasBeforeValue ? *Diff.BeforeValue.CanonicalValueText : TEXT("<없음>"),
				Diff.bHasAfterValue ? *Diff.AfterValue.CanonicalValueText : TEXT("<없음>"));
		}
	}

	Summary += TEXT(
		"\n\nStep 7 경계:"
		"\n- Review는 R0 read-only"
		"\n- Apply는 USER explicit DefinitionApply 승인 뒤 existing R3 Apply lane"
		"\n- Apply 성공 시 Target VehicleData와 Recipe AppliedState가 transaction으로 변경될 수 있음"
		"\n- 자동 Save / 자동 재시도 없음"
		"\n- Undo는 이 Builder Apply가 만든 exact UE transaction top + post-Apply state가 그대로일 때만 허용");
	return Summary;
}

// Step 7의 current Final Review를 fresh mutation0로 다시 읽고 exact DefinitionApply proposal을 USER dialog 직전 prepared state로 보관합니다.
bool FCFVehicleBuilderVM::PrepareFinalReviewApply(
	FCFBuilderFinalReviewResult& OutReview,
	FString& OutError)
{
	ClearPreparedFinalReviewApply();

	// Apply review 직전 prerequisite diagnostic입니다.
	FString PrerequisiteError;
	if (!AreFinalReviewPrerequisitesComplete(PrerequisiteError))
	{
		OutError = PrerequisiteError;
		return false;
	}

	// Existing Final Review facade에 전달할 exact current request입니다.
	FCFBuilderFinalReviewRequest Request;
	if (!BuildFinalReviewRequest(Request, OutError))
	{
		return false;
	}

	// USER dialog 직전 fresh R0 Final Review입니다.
	FCFBuilderFinalReviewResult Review;
	if (!FCFVehicleAuthoringService::ReadBuilderFinalReview(Request, Review))
	{
		FinalReviewResult = Review;
		bHasFinalReviewResult = true;
		OutError = Review.Operation.Message.IsEmpty()
			? TEXT("Final Review mutation0 read에 실패했습니다.")
			: Review.Operation.Message;
		return false;
	}

	FinalReviewResult = Review;
	bHasFinalReviewResult = true;
	OutReview = Review;
	if (!Review.bCanApply || Review.ApplyProposal.ProposalHash.IsEmpty())
	{
		OutError = Review.bCanCompleteFinalReview
			? TEXT("Current Final Review는 이미 Complete이며 적용할 Target Diff가 없습니다.")
			: Review.Operation.Message.IsEmpty()
				? TEXT("Current Final Review가 DefinitionApply 가능한 상태가 아닙니다.")
				: Review.Operation.Message;
		return false;
	}

	PreparedFinalReviewRequest = Request;
	PreparedFinalReviewResult = Review;
	bHasPreparedFinalReviewApply = true;
	OutError.Reset();
	return true;
}

// 직전 exact Final Review proposal에 USER DefinitionApply approval을 붙여 existing R3 Apply lane을 실행합니다.
bool FCFVehicleBuilderVM::ExecutePreparedFinalReviewApply(
	FCFBuilderFinalApplyResult& OutResult,
	FString& OutError)
{
	if (!bHasPreparedFinalReviewApply || PreparedFinalReviewResult.ApplyProposal.ProposalHash.IsEmpty())
	{
		OutError = TEXT("먼저 Final Review의 fresh DefinitionApply proposal을 검토해야 합니다.");
		return false;
	}

	// USER가 확인한 exact Final Review request입니다.
	const FCFBuilderFinalReviewRequest ApprovedReviewRequest = PreparedFinalReviewRequest;
	// USER가 확인한 exact DefinitionApply proposal hash입니다.
	const FString ApprovedProposalHash = PreparedFinalReviewResult.ApplyProposal.ProposalHash;
	ClearPreparedFinalReviewApply();

	// Existing R3 facade가 fresh re-review할 explicit Apply request입니다.
	FCFBuilderFinalApplyRequest ApplyRequest;
	ApplyRequest.ReviewRequest = ApprovedReviewRequest;
	ApplyRequest.CallContext.ClientOperationId = FString::Printf(
		TEXT("Builder-FinalApply-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower));
	ApplyRequest.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	ApplyRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	ApplyRequest.CallContext.ApprovalScopeHash = ApprovedProposalHash;

	if (!FCFVehicleAuthoringService::ApplyBuilderFinalReview(ApplyRequest, OutResult))
	{
		// Stale approval 등 실패 뒤 UI는 old review를 유지하지 않고 current truth를 다시 읽습니다.
		FString RefreshError;
		RefreshCurrentState(RefreshError);
		OutError = OutResult.Operation.Message.IsEmpty()
			? (RefreshError.IsEmpty() ? TEXT("Final Review DefinitionApply가 차단됐습니다.") : RefreshError)
			: OutResult.Operation.Message;
		return false;
	}

	if (OutResult.bUndoAvailable)
	{
		FinalReviewUndoToken = OutResult.UndoToken;
		bHasFinalReviewUndoToken = true;
	}
	else
	{
		ClearFinalReviewUndoToken();
	}

	// Apply terminal success 뒤 Step 7 Complete/diff0를 current truth에서 다시 파생합니다.
	FString RefreshError;
	if (!RefreshCurrentState(RefreshError))
	{
		OutError = FString::Printf(
			TEXT("DefinitionApply는 성공했지만 post-Apply Final Review refresh에 실패했습니다: %s"),
			*RefreshError);
		return true;
	}

	OutError.Reset();
	return true;
}

// USER explicit DefinitionApply approval로 exact Builder-owned top transaction guarded Undo를 실행합니다.
bool FCFVehicleBuilderVM::ExecuteFinalReviewUndo(
	FCFAuthoringOpResult& OutResult,
	FString& OutError)
{
	if (!bHasFinalReviewUndoToken || !FinalReviewUndoToken.TransactionId.IsValid())
	{
		OutError = TEXT("현재 Builder가 안전하게 되돌릴 guarded Undo token이 없습니다.");
		return false;
	}

	// USER가 확인한 current exact guarded Undo token입니다.
	const FCFBuilderUndoToken ApprovedUndoToken = FinalReviewUndoToken;
	// Existing guarded Undo facade가 fresh top/post-state를 검사할 explicit request입니다.
	FCFBuilderUndoRequest UndoRequest;
	UndoRequest.UndoToken = ApprovedUndoToken;
	UndoRequest.CallContext.ClientOperationId = FString::Printf(
		TEXT("Builder-FinalUndo-%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower));
	UndoRequest.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	UndoRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	UndoRequest.CallContext.ApprovalScopeHash = ApprovedUndoToken.UndoScopeHash;

	if (!FCFVehicleAuthoringService::UndoBuilderFinalApply(UndoRequest, OutResult))
	{
		// 실제 Undo가 실행된 뒤 post-readback만 실패한 경우 token은 backend에서 이미 소비됐으므로 local token도 폐기합니다.
		if (OutResult.Mutation.bTargetChanged || OutResult.Mutation.bRecipeChanged)
		{
			ClearFinalReviewUndoToken();
			FString RefreshError;
			RefreshCurrentState(RefreshError);
		}
		OutError = OutResult.Message.IsEmpty()
			? TEXT("Builder guarded Undo가 current transaction/state guard에서 차단됐습니다.")
			: OutResult.Message;
		return false;
	}

	ClearFinalReviewUndoToken();

	// Undo 성공 뒤 pre-Apply diff/readiness를 fresh Final Review에서 복원합니다.
	FString RefreshError;
	if (!RefreshCurrentState(RefreshError))
	{
		OutError = FString::Printf(
			TEXT("Guarded Undo는 성공했지만 post-Undo Final Review refresh에 실패했습니다: %s"),
			*RefreshError);
		return true;
	}

	OutError.Reset();
	return true;
}

// Final Review 단계의 existing R0 review / explicit R3 Apply / guarded Undo authority를 fresh 평가합니다.
void FCFVehicleBuilderVM::EvaluateFinalReviewStep()
{
	FinalReviewResult = FCFBuilderFinalReviewResult();
	bHasFinalReviewResult = false;

	// Step 7은 Gameplay Setup이 current Complete일 때만 Final Review를 평가합니다.
	FString PrerequisiteError;
	if (!AreFinalReviewPrerequisitesComplete(PrerequisiteError))
	{
		SetStep(
			ECFVehicleBuilderStepId::FinalReview,
			ECFVehicleBuilderStepState::Locked,
			PrerequisiteError,
			TEXT("Step 6 Gameplay Setup을 current Complete 상태로 만든 뒤 다시 확인하세요."),
			true);
		return;
	}

	// Existing Final Review facade에 전달할 exact Recipe/Target/Evidence request입니다.
	FCFBuilderFinalReviewRequest Request;
	FString RequestError;
	if (!BuildFinalReviewRequest(Request, RequestError))
	{
		SetStep(
			ECFVehicleBuilderStepId::FinalReview,
			ECFVehicleBuilderStepState::Blocked,
			RequestError,
			TEXT("Current Reference Evidence/Builder receipt/Recipe/Target binding을 복구한 뒤 fresh review하세요."),
			true);
		return;
	}

	// Existing R0 aggregate Final Review result입니다.
	FCFBuilderFinalReviewResult Review;
	if (!FCFVehicleAuthoringService::ReadBuilderFinalReview(Request, Review))
	{
		FinalReviewResult = Review;
		bHasFinalReviewResult = true;
		SetStep(
			ECFVehicleBuilderStepId::FinalReview,
			ECFVehicleBuilderStepState::Blocked,
			Review.Operation.Message.IsEmpty()
				? TEXT("Final Review R0 read가 current authoritative truth를 평가하지 못했습니다.")
				: Review.Operation.Message,
			TEXT("Validation/Drift/Gameplay/Provenance 상태를 복구한 뒤 다시 확인하세요. 자동 Apply하지 않습니다."),
			true);
		return;
	}

	FinalReviewResult = MoveTemp(Review);
	bHasFinalReviewResult = true;

	if (FinalReviewResult.BlockingIssueCount > 0 || FinalReviewResult.bHasExternalDrift)
	{
		SetStep(
			ECFVehicleBuilderStepId::FinalReview,
			ECFVehicleBuilderStepState::Blocked,
			FString::Printf(
				TEXT("Final Review에 blocker %d개가 있으며 External Drift=%s입니다. Target Diff=%d."),
				FinalReviewResult.BlockingIssueCount,
				FinalReviewResult.bHasExternalDrift ? TEXT("있음") : TEXT("없음"),
				FinalReviewResult.FieldDiff.Num()),
			TEXT("Step 7 상세의 validation/drift/provenance 원인을 해결한 뒤 fresh review하세요. blocker를 우회해 Apply하지 않습니다."),
			true);
		return;
	}

	if (FinalReviewResult.bCanCompleteFinalReview)
	{
		SetStep(
			ECFVehicleBuilderStepId::FinalReview,
			ECFVehicleBuilderStepState::Complete,
			FString::Printf(
				TEXT("Final Review PASS입니다. Warning %d / Blocker 0 / Target Diff 0."),
				FinalReviewResult.WarningCount),
			bHasFinalReviewUndoToken
				? TEXT("Apply 결과가 current이며 Step 8로 진행할 수 있습니다. 필요하면 exact guarded Undo로 마지막 Builder Apply만 되돌릴 수 있습니다.")
				: TEXT("Target Definition이 current Authoring truth와 일치합니다. Step 8 Driving Test로 진행할 수 있습니다."),
			true);
		return;
	}

	if (FinalReviewResult.bCanApply)
	{
		SetStep(
			ECFVehicleBuilderStepId::FinalReview,
			ECFVehicleBuilderStepState::Ready,
			FString::Printf(
				TEXT("Final Review는 Apply 준비됐습니다. Warning %d / Blocker 0 / Target Diff %d."),
				FinalReviewResult.WarningCount,
				FinalReviewResult.FieldDiff.Num()),
			TEXT("상세 Field Diff와 Reference provenance를 검토한 뒤 'Final Review 검토 후 적용'을 눌러 explicit DefinitionApply를 승인하세요."),
			true);
		return;
	}

	SetStep(
		ECFVehicleBuilderStepId::FinalReview,
		ECFVehicleBuilderStepState::Blocked,
		FString::Printf(
			TEXT("Final Review가 blocker 없이도 Apply/Complete 상태로 확정되지 않았습니다. Target Diff=%d."),
			FinalReviewResult.FieldDiff.Num()),
		TEXT("Current Final Review result를 확인하고 backend contract mismatch로 취급하세요. 자동 우회하지 않습니다."),
		true);
}


// Step 8이 사용하는 existing VB-P0-08 benchmark result JSON path를 반환합니다.
FString FCFVehicleBuilderVM::GetDrivingBenchmarkResultPath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CarFight"), TEXT("VehicleBuilderBenchmarkResult.json"));
}

// Step 8이 요구하는 Step 7 current Complete prerequisite를 검사합니다.
bool FCFVehicleBuilderVM::AreDrivingTestPrerequisitesComplete(FString& OutError) const
{
	// Step 8의 immediate prerequisite인 Final Review projection입니다.
	const FCFVehicleBuilderStepView* FinalReviewStep = FindStepView(ECFVehicleBuilderStepId::FinalReview);
	if (!FinalReviewStep || FinalReviewStep->State != ECFVehicleBuilderStepState::Complete)
	{
		OutError = FinalReviewStep
			? FString::Printf(
				TEXT("Driving Test 전에 Final Review가 Complete여야 합니다. 현재 상태: %s"),
				StepStateText(FinalReviewStep->State))
			: TEXT("Driving Test prerequisite Final Review Step을 찾을 수 없습니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// Current saved Target object path와 exact current DefinitionHash를 Step 8 identity로 읽습니다.
bool FCFVehicleBuilderVM::BuildDrivingTargetIdentity(
	FSoftObjectPath& OutTargetPath,
	FString& OutTargetDefinitionHash,
	FString& OutError) const
{
	OutTargetPath.Reset();
	OutTargetDefinitionHash.Reset();

	// Step 8 identity owner인 current managed Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || IsMeshOnlyCandidate() || !Recipe->RecipeId.IsValid())
	{
		OutError = TEXT("Driving Test에 사용할 valid managed Recipe가 없습니다.");
		return false;
	}

	// 실제 benchmark/test-drive target VehicleData입니다.
	UCFVehicleData* TargetVehicleData = Recipe->TargetVehicleData.Get();
	if (!TargetVehicleData)
	{
		TargetVehicleData = Recipe->TargetVehicleData.LoadSynchronous();
	}
	if (!TargetVehicleData)
	{
		OutError = TEXT("Driving Test에 사용할 Target VehicleData를 load할 수 없습니다.");
		return false;
	}

	if (!AuthoringViewModel.IsValid())
	{
		OutError = TEXT("Driving Test current DefinitionHash를 읽을 Authoring ViewModel이 없습니다.");
		return false;
	}

	// Fresh Authoring preview가 소유하는 current Target semantic snapshot입니다.
	const FCFVehicleResolveReadResult& ResolveRead = AuthoringViewModel->GetResolveResult();
	if (ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash.IsEmpty())
	{
		OutError = TEXT("Driving Test current Target DefinitionHash가 비어 있습니다. 현재 상태를 다시 확인하세요.");
		return false;
	}

	OutTargetPath = FSoftObjectPath(TargetVehicleData);
	OutTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
	OutError.Reset();
	return true;
}

// Current saved Target exact path/hash를 binding한 VB-P0-08 runner process launch 정보를 만듭니다.
bool FCFVehicleBuilderVM::PrepareDrivingBenchmarkLaunch(
	FString& OutExecutable,
	FString& OutArguments,
	FString& OutWorkingDirectory,
	FString& OutRunId,
	FString& OutError)
{
	OutExecutable.Reset();
	OutArguments.Reset();
	OutWorkingDirectory.Reset();
	OutRunId.Reset();

	// Benchmark launch 직전 Step 7 completion diagnostic입니다.
	FString PrerequisiteError;
	if (!AreDrivingTestPrerequisitesComplete(PrerequisiteError))
	{
		OutError = PrerequisiteError;
		return false;
	}

	// Current Target exact object path입니다.
	FSoftObjectPath TargetPath;
	// Current Target exact semantic DefinitionHash입니다.
	FString TargetDefinitionHash;
	if (!BuildDrivingTargetIdentity(TargetPath, TargetDefinitionHash, OutError))
	{
		return false;
	}

	// Current Recipe/Target persistent save gate를 검사할 Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// Current benchmark target입니다.
	UCFVehicleData* TargetVehicleData = Recipe ? Recipe->TargetVehicleData.LoadSynchronous() : nullptr;
	if (!Recipe || !TargetVehicleData)
	{
		OutError = TEXT("Benchmark saved-state gate에서 Recipe/Target을 load할 수 없습니다.");
		return false;
	}

	if (Recipe->GetOutermost()->IsDirty() || TargetVehicleData->GetOutermost()->IsDirty())
	{
		OutError = TEXT("Technical Benchmark는 saved VehicleData만 읽습니다. Step 7 Apply 뒤 Recipe와 Target VehicleData를 직접 저장한 후 다시 실행하세요. Builder는 자동 저장하지 않습니다.");
		return false;
	}

	if (!FPackageName::DoesPackageExist(Recipe->GetOutermost()->GetName())
		|| !FPackageName::DoesPackageExist(TargetVehicleData->GetOutermost()->GetName()))
	{
		OutError = TEXT("Technical Benchmark 전에 Recipe와 Target VehicleData가 disk에 저장되어 있어야 합니다.");
		return false;
	}

	if (Recipe->AppliedState.AppliedDefinitionHash.IsEmpty()
		|| Recipe->AppliedState.AppliedDefinitionHash != TargetDefinitionHash)
	{
		OutError = TEXT("Current Target DefinitionHash와 Recipe AppliedState가 일치하지 않습니다. Final Review를 fresh 확인한 뒤 다시 시도하세요.");
		return false;
	}

	// CarFight repository root입니다.
	const FString RepositoryRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("..")));
	// Existing VB-P0-08 canonical runner script입니다.
	const FString ScriptPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(RepositoryRoot, TEXT("Tools"), TEXT("RunBuilderBench.ps1")));
	if (!FPaths::FileExists(ScriptPath))
	{
		OutError = FString::Printf(TEXT("VB-P0-08 benchmark runner를 찾을 수 없습니다: %s"), *ScriptPath);
		return false;
	}

	// 이번 benchmark result와 USER acceptance를 binding할 fresh run identity입니다.
	OutRunId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	// 기존 runner process를 시작할 Windows PowerShell executable입니다.
	OutExecutable = TEXT("powershell.exe");
	// Existing script가 canonical UE 5.8 fixed-60Hz benchmark를 실행하도록 전달할 exact arguments입니다.
	OutArguments = FString::Printf(
		TEXT("-NoProfile -ExecutionPolicy Bypass -File %s -VehicleDataPath %s -Label %s -RunId %s -ExpectedTargetDefinitionHash %s"),
		*QuotePowerShellArgument(ScriptPath),
		*QuotePowerShellArgument(TargetPath.ToString()),
		*QuotePowerShellArgument(Recipe->GetName()),
		*QuotePowerShellArgument(OutRunId),
		*QuotePowerShellArgument(TargetDefinitionHash));
	OutWorkingDirectory = RepositoryRoot;

	// 새 benchmark run을 시작하면 current session USER test-drive 준비는 다시 요구합니다.
	bUserTestDrivePreparedThisSession = false;
	DrivingBenchmarkResult = FCFVehicleBuilderBenchmarkResult();
	bHasDrivingBenchmarkResult = false;
	DrivingBenchmarkStateError = TEXT("Technical Driving Benchmark 실행 중입니다.");
	OutError.Reset();
	return true;
}

// Existing VB-P0-08 JSON을 읽고 current Target path/hash에 exact binding된 result만 current로 인정합니다.
bool FCFVehicleBuilderVM::RefreshDrivingBenchmarkState(FString& OutError)
{
	// 이전 current result RunId입니다. 새 run 도착 시 USER test-drive session readiness를 무효화하는 데 사용합니다.
	const FString PreviousRunId = bHasDrivingBenchmarkResult ? DrivingBenchmarkResult.RunId : FString();

	DrivingBenchmarkResult = FCFVehicleBuilderBenchmarkResult();
	bHasDrivingBenchmarkResult = false;
	DrivingBenchmarkStateError.Reset();

	// Current Target exact identity입니다.
	FSoftObjectPath TargetPath;
	// Current Target exact semantic hash입니다.
	FString TargetDefinitionHash;
	if (!BuildDrivingTargetIdentity(TargetPath, TargetDefinitionHash, OutError))
	{
		DrivingBenchmarkStateError = OutError;
		return false;
	}

	// Existing runner의 single result JSON입니다.
	const FString ResultPath = GetDrivingBenchmarkResultPath();
	if (!FPaths::FileExists(ResultPath))
	{
		OutError.Reset();
		return true;
	}

	// UTF-8 JSON 원문입니다.
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ResultPath))
	{
		DrivingBenchmarkStateError = FString::Printf(TEXT("Technical Benchmark result를 읽을 수 없습니다: %s"), *ResultPath);
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	// Parsed JSON object입니다.
	TSharedPtr<FJsonObject> RootObject;
	// JSON reader입니다.
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark result JSON 형식이 올바르지 않습니다.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	// Parsed result envelope입니다.
	FCFVehicleBuilderBenchmarkResult ParsedResult;
	if (!RootObject->TryGetStringField(TEXT("schema_version"), ParsedResult.SchemaVersion)
		|| !RootObject->TryGetStringField(TEXT("status"), ParsedResult.Status)
		|| !RootObject->TryGetStringField(TEXT("run_id"), ParsedResult.RunId)
		|| !RootObject->TryGetStringField(TEXT("expected_target_definition_hash"), ParsedResult.ExpectedTargetDefinitionHash)
		|| !RootObject->TryGetStringField(TEXT("completed_utc"), ParsedResult.CompletedUtc)
		|| !RootObject->TryGetBoolField(TEXT("reference_threshold_asserted"), ParsedResult.bReferenceThresholdAsserted)
		|| !RootObject->TryGetBoolField(TEXT("user_driving_feel_asserted"), ParsedResult.bUserDrivingFeelAsserted))
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark result에 Step 8 identity 필드가 부족합니다. current Guided runner로 다시 실행하세요.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	if (!RootObject->HasTypedField<EJson::Object>(TEXT("metric")))
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark result에 exact metric object가 없습니다.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	// Existing VB-P0-08 metric object입니다.
	const TSharedPtr<FJsonObject> MetricObject = RootObject->GetObjectField(TEXT("metric"));
	if (!MetricObject.IsValid())
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark metric object를 읽을 수 없습니다.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	// Parsed metric projection입니다.
	FCFVehicleBuilderDrivingMetric& Metric = ParsedResult.Metric;
	if (!MetricObject->TryGetStringField(TEXT("label"), Metric.Label)
		|| !MetricObject->TryGetStringField(TEXT("vehicle_data_path"), Metric.VehicleDataPath)
		|| !MetricObject->TryGetStringField(TEXT("fitting_data_path"), Metric.FittingDataPath)
		|| !MetricObject->TryGetNumberField(TEXT("configured_mass_kg"), Metric.ConfiguredMassKg)
		|| !MetricObject->TryGetNumberField(TEXT("actual_mass_kg"), Metric.ActualMassKg)
		|| !MetricObject->TryGetNumberField(TEXT("acceleration_0_to_50_sec"), Metric.Acceleration0To50Seconds)
		|| !MetricObject->TryGetNumberField(TEXT("acceleration_0_to_100_sec"), Metric.Acceleration0To100Seconds)
		|| !MetricObject->TryGetNumberField(TEXT("peak_speed_kmh"), Metric.PeakSpeedKmh)
		|| !MetricObject->TryGetBoolField(TEXT("top_speed_stable"), Metric.bTopSpeedStable)
		|| !MetricObject->TryGetNumberField(TEXT("peak_engine_rpm"), Metric.PeakEngineRpm)
		|| !MetricObject->TryGetNumberField(TEXT("peak_speed_gear"), Metric.PeakSpeedGear)
		|| !MetricObject->TryGetBoolField(TEXT("braking_100_available"), Metric.bBraking100Available)
		|| !MetricObject->TryGetNumberField(TEXT("braking_start_kmh"), Metric.BrakingStartKmh)
		|| !MetricObject->TryGetNumberField(TEXT("braking_100_to_idle_sec"), Metric.Braking100ToIdleSeconds)
		|| !MetricObject->TryGetNumberField(TEXT("braking_100_to_idle_distance_m"), Metric.Braking100ToIdleDistanceMeters)
		|| !MetricObject->TryGetNumberField(TEXT("steady_yaw_deg"), Metric.SteadyYawDegrees)
		|| !MetricObject->TryGetNumberField(TEXT("effective_turning_radius_m"), Metric.EffectiveTurningRadiusMeters)
		|| !MetricObject->TryGetNumberField(TEXT("turning_average_speed_kmh"), Metric.TurningAverageSpeedKmh))
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark metric 필드가 current VB-P0-08 schema와 맞지 않습니다.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}
	ParsedResult.bHasMetric = true;
	DrivingBenchmarkResult = ParsedResult;

	if (ParsedResult.SchemaVersion != TEXT("carfight_vehicle_builder_benchmark_v1")
		|| ParsedResult.Status != TEXT("success")
		|| ParsedResult.RunId.IsEmpty()
		|| ParsedResult.ExpectedTargetDefinitionHash.IsEmpty())
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark result status/schema/identity가 current Guided Step 8 contract를 만족하지 않습니다.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	if (ParsedResult.bReferenceThresholdAsserted || ParsedResult.bUserDrivingFeelAsserted)
	{
		DrivingBenchmarkStateError = TEXT("Technical runner가 Reference threshold 또는 USER Driving Feel을 대신 판정한 결과는 Step 8에서 인정하지 않습니다.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	if (Metric.VehicleDataPath != TargetPath.ToString())
	{
		DrivingBenchmarkStateError = FString::Printf(
			TEXT("Technical Benchmark 대상이 current Target과 다릅니다. Result=%s Current=%s"),
			*Metric.VehicleDataPath,
			*TargetPath.ToString());
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	if (ParsedResult.ExpectedTargetDefinitionHash != TargetDefinitionHash)
	{
		DrivingBenchmarkStateError = TEXT("Technical Benchmark가 binding한 Target DefinitionHash가 current Target과 달라졌습니다. current 차량으로 다시 실행하세요.");
		OutError = DrivingBenchmarkStateError;
		return false;
	}

	bHasDrivingBenchmarkResult = true;
	if (!PreviousRunId.IsEmpty() && PreviousRunId != ParsedResult.RunId)
	{
		bUserTestDrivePreparedThisSession = false;
	}
	OutError.Reset();
	return true;
}

// USER-facing Step 8 technical metric / saved-state / USER Driving acceptance 요약을 만듭니다.
FString FCFVehicleBuilderVM::BuildDrivingTestSummary() const
{
	// Current managed Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	// Current Target VehicleData입니다.
	const UCFVehicleData* TargetVehicleData = Recipe ? Recipe->TargetVehicleData.Get() : nullptr;
	// Persistent package saved-state diagnostic입니다.
	const bool bRecipeDirty = Recipe && Recipe->GetOutermost()->IsDirty();
	// Persistent Target package saved-state diagnostic입니다.
	const bool bTargetDirty = TargetVehicleData && TargetVehicleData->GetOutermost()->IsDirty();

	FString Summary = FString::Printf(
		TEXT(
			"Saved-state gate\n"
			"- Recipe Dirty: %s\n"
			"- Target VehicleData Dirty: %s\n"
			"- Builder Auto Save: 안 함\n\n"),
		bRecipeDirty ? TEXT("예") : TEXT("아니오"),
		bTargetDirty ? TEXT("예") : TEXT("아니오"));

	if (!bHasDrivingBenchmarkResult)
	{
		Summary += DrivingBenchmarkStateError.IsEmpty()
			? TEXT("Technical Benchmark: current Target에 binding된 결과 없음\n")
			: FString::Printf(TEXT("Technical Benchmark: current 결과 미인정\n%s\n"), *DrivingBenchmarkStateError);
		Summary += TEXT(
			"Step 7 Apply 뒤 Recipe와 Target을 직접 저장하고 '기술 벤치마크 실행'을 사용하세요.\n"
			"Runner는 fixed 60Hz fresh PIE에서 수치만 계측하며 실차 threshold나 주행감을 임의 PASS/FAIL하지 않습니다.");
		return Summary;
	}

	// Current technical benchmark metric입니다.
	const FCFVehicleBuilderDrivingMetric& Metric = DrivingBenchmarkResult.Metric;
	// 0→100 미도달을 기술 실패가 아니라 관측 결과로 표시합니다.
	const FString Accel100Text = Metric.Acceleration0To100Seconds >= 0.0
		? FString::Printf(TEXT("%.3f s"), Metric.Acceleration0To100Seconds)
		: TEXT("미도달");
	// 100km/h braking 미수행을 차량 관측 결과로 표시합니다.
	const FString BrakingText = Metric.bBraking100Available
		? FString::Printf(TEXT("%.3f s / %.3f m"), Metric.Braking100ToIdleSeconds, Metric.Braking100ToIdleDistanceMeters)
		: TEXT("100km/h 미도달로 미측정");

	Summary += FString::Printf(
		TEXT(
			"Technical Benchmark: CURRENT\n"
			"- RunId: %s\n"
			"- TargetHash: %s\n"
			"- Mass: configured %.3f kg / actual %.3f kg\n"
			"- 0→50: %.3f s\n"
			"- 0→100: %s\n"
			"- Peak Speed: %.3f km/h | stable: %s\n"
			"- Peak RPM / Gear: %.1f / %d\n"
			"- 100→Idle Brake: %s\n"
			"- Steady Yaw: %.3f deg\n"
			"- Effective Turning Radius: %.3f m\n"
			"- Reference threshold asserted: 아니오\n"
			"- USER driving feel asserted: 아니오\n\n"),
		*DrivingBenchmarkResult.RunId,
		*DrivingBenchmarkResult.ExpectedTargetDefinitionHash,
		Metric.ConfiguredMassKg,
		Metric.ActualMassKg,
		Metric.Acceleration0To50Seconds,
		*Accel100Text,
		Metric.PeakSpeedKmh,
		Metric.bTopSpeedStable ? TEXT("예") : TEXT("아니오"),
		Metric.PeakEngineRpm,
		Metric.PeakSpeedGear,
		*BrakingText,
		Metric.SteadyYawDegrees,
		Metric.EffectiveTurningRadiusMeters);

	Summary += FString::Printf(
		TEXT(
			"USER Driving\n"
			"- Active PIE에 선택 차량 transient 적용 준비: %s\n"
			"- 이 exact Target Definition USER PASS: %s\n\n"
			"직접 확인할 것:\n"
			"1. 출발/가속 반응이 Reference와 의도한 차량 성격에 어울리는가\n"
			"2. 조향 반응과 회전반경이 차량 크기/성격에 어울리는가\n"
			"3. 제동이 정상적이고 조작 가능한가\n"
			"4. RPM/변속/고속 반응에 명백한 이상이 없는가\n"
			"5. Wheel/차체 물리에 플레이를 막는 이상이 없는가\n\n"
			"수치 benchmark는 판단 보조이며 USER 주행감 PASS를 대체하지 않습니다."),
		bUserTestDrivePreparedThisSession ? TEXT("예") : TEXT("아니오"),
		HasCurrentUserDrivingAcceptance() ? TEXT("예") : TEXT("아니오"));
	return Summary;
}

// Active PIE player VehiclePawn에 current selected saved VehicleData의 transient duplicate를 적용해 USER test-drive를 준비합니다.
bool FCFVehicleBuilderVM::ApplySelectedVehicleToActivePIE(FString& OutError)
{
	if (!bHasDrivingBenchmarkResult)
	{
		OutError = TEXT("USER Driving 전에 current Target에 exact binding된 Technical Benchmark가 필요합니다.");
		return false;
	}

	// Current target exact identity를 다시 확인합니다.
	FSoftObjectPath TargetPath;
	// Current target exact hash입니다.
	FString TargetDefinitionHash;
	if (!BuildDrivingTargetIdentity(TargetPath, TargetDefinitionHash, OutError)
		|| TargetDefinitionHash != DrivingBenchmarkResult.ExpectedTargetDefinitionHash)
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("USER Driving target hash가 benchmark와 달라졌습니다.");
		}
		return false;
	}

	// Persistent target source입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// Persistent target VehicleData입니다.
	UCFVehicleData* PersistentTarget = Recipe ? Recipe->TargetVehicleData.LoadSynchronous() : nullptr;
	if (!Recipe || !PersistentTarget)
	{
		OutError = TEXT("USER Driving에 사용할 current Recipe/Target을 load할 수 없습니다.");
		return false;
	}

	if (Recipe->GetOutermost()->IsDirty() || PersistentTarget->GetOutermost()->IsDirty())
	{
		OutError = TEXT("USER Driving은 current Technical Benchmark와 같은 saved Target을 사용해야 합니다. Recipe/Target을 저장한 뒤 다시 확인하세요.");
		return false;
	}

	// Active PIE world입니다. Editor world에는 적용하지 않습니다.
	UWorld* PIEWorld = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
			{
				PIEWorld = WorldContext.World();
				break;
			}
		}
	}
	if (!PIEWorld)
	{
		OutError = TEXT("활성 PIE가 없습니다. Unreal에서 플레이(Play)를 시작한 뒤 이 버튼을 다시 누르세요.");
		return false;
	}

	// 실제 USER가 조종 중인 local PlayerController입니다.
	APlayerController* PlayerController = PIEWorld->GetFirstPlayerController();
	if (!PlayerController)
	{
		OutError = TEXT("Active PIE의 PlayerController를 찾을 수 없습니다.");
		return false;
	}

	// 선택 VehicleData를 적용할 실제 CarFight player VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = Cast<ACFVehiclePawn>(PlayerController->GetPawn());
	if (!VehiclePawn)
	{
		OutError = TEXT("Active PIE Player Pawn이 CFVehiclePawn이 아닙니다.");
		return false;
	}

	// Persistent Asset을 runtime mutation에서 격리할 transient duplicate 이름입니다.
	const FName TransientVehicleDataName = MakeUniqueObjectName(
		GetTransientPackage(),
		UCFVehicleData::StaticClass(),
		TEXT("VB_UserDriveVehicleData"));
	// Current saved Target의 value-equivalent transient runtime copy입니다.
	UCFVehicleData* TransientVehicleData = DuplicateObject<UCFVehicleData>(
		PersistentTarget,
		GetTransientPackage(),
		TransientVehicleDataName);
	if (!TransientVehicleData)
	{
		OutError = TEXT("USER Driving용 transient VehicleData를 만들 수 없습니다.");
		return false;
	}
	TransientVehicleData->ClearFlags(RF_Public | RF_Standalone);
	TransientVehicleData->SetFlags(RF_Transient);

	// Current Step 8 기본 flow는 VehicleData BaseMass benchmark와 동일하게 별도 Fitting override를 사용하지 않습니다.
	VehiclePawn->VehicleFittingData = nullptr;
	VehiclePawn->VehicleData = TransientVehicleData;
	if (!VehiclePawn->InitializeVehicleRuntime())
	{
		OutError = TEXT("선택 VehicleData를 Active PIE VehiclePawn runtime에 초기화하지 못했습니다.");
		return false;
	}

	bUserTestDrivePreparedThisSession = true;
	OutError.Reset();
	return true;
}

// Current Recipe별 USER Driving PASS local token을 EditorPerProject settings에서 복원합니다.
void FCFVehicleBuilderVM::LoadDrivingAcceptanceToken()
{
	ClearDrivingAcceptanceToken();

	// Token owner인 current Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !Recipe->RecipeId.IsValid() || !GConfig)
	{
		return;
	}

	// Recipe별 local acceptance section입니다.
	const FString Section = BuildDrivingAcceptanceSection(Recipe->RecipeId);
	// Stored RecipeId 문자열입니다.
	FString RecipeIdText;
	// Stored Target DefinitionHash입니다.
	FString TargetDefinitionHash;
	// Stored benchmark RunId입니다.
	FString BenchmarkRunId;
	if (!GConfig->GetString(*Section, TEXT("RecipeId"), RecipeIdText, GEditorPerProjectIni)
		|| !GConfig->GetString(*Section, TEXT("TargetDefinitionHash"), TargetDefinitionHash, GEditorPerProjectIni)
		|| !GConfig->GetString(*Section, TEXT("BenchmarkRunId"), BenchmarkRunId, GEditorPerProjectIni))
	{
		return;
	}

	// Parsed local Recipe identity입니다.
	FGuid ParsedRecipeId;
	if (!FGuid::Parse(RecipeIdText, ParsedRecipeId)
		|| ParsedRecipeId != Recipe->RecipeId
		|| TargetDefinitionHash.IsEmpty()
		|| BenchmarkRunId.IsEmpty())
	{
		return;
	}

	AcceptedDrivingRecipeId = ParsedRecipeId;
	AcceptedDrivingTargetDefinitionHash = MoveTemp(TargetDefinitionHash);
	AcceptedDrivingBenchmarkRunId = MoveTemp(BenchmarkRunId);
}

// Current benchmark run + Target hash USER Driving PASS token을 EditorPerProject settings에 저장합니다.
void FCFVehicleBuilderVM::SaveDrivingAcceptanceToken() const
{
	if (!GConfig
		|| !AcceptedDrivingRecipeId.IsValid()
		|| AcceptedDrivingTargetDefinitionHash.IsEmpty()
		|| AcceptedDrivingBenchmarkRunId.IsEmpty())
	{
		return;
	}

	// Recipe별 local acceptance section입니다.
	const FString Section = BuildDrivingAcceptanceSection(AcceptedDrivingRecipeId);
	// Stable RecipeId string입니다.
	const FString RecipeIdText = AcceptedDrivingRecipeId.ToString(EGuidFormats::Digits);
	GConfig->SetString(*Section, TEXT("RecipeId"), *RecipeIdText, GEditorPerProjectIni);
	GConfig->SetString(*Section, TEXT("TargetDefinitionHash"), *AcceptedDrivingTargetDefinitionHash, GEditorPerProjectIni);
	GConfig->SetString(*Section, TEXT("BenchmarkRunId"), *AcceptedDrivingBenchmarkRunId, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

// Selection 전환에서 transient USER Driving token state를 비웁니다.
void FCFVehicleBuilderVM::ClearDrivingAcceptanceToken()
{
	AcceptedDrivingRecipeId.Invalidate();
	AcceptedDrivingTargetDefinitionHash.Reset();
	AcceptedDrivingBenchmarkRunId.Reset();
}

// Current Step 8 USER Driving PASS가 persistent Target Definition receipt 또는 legacy local token으로 current Target에 일치하는지 반환합니다.
bool FCFVehicleBuilderVM::HasCurrentUserDrivingAcceptance() const
{
	if (!bHasDrivingBenchmarkResult)
	{
		return false;
	}

	// Current Recipe입니다.
	const UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe)
	{
		return false;
	}

	// Current Target identity입니다.
	FSoftObjectPath TargetPath;
	// Current Target exact hash입니다.
	FString TargetDefinitionHash;
	// Read-only identity diagnostic입니다.
	FString IdentityError;
	if (!BuildDrivingTargetIdentity(TargetPath, TargetDefinitionHash, IdentityError)
		|| DrivingBenchmarkResult.ExpectedTargetDefinitionHash != TargetDefinitionHash)
	{
		return false;
	}

	// 새 durable authority: USER는 benchmark invocation이 아니라 exact Vehicle Definition을 주행해 PASS합니다.
	const FCFVehicleBuilderDrivingAcceptanceReceipt& Receipt = Recipe->BuilderDrivingAcceptanceReceipt;
	if (Receipt.IsValid()
		&& Receipt.TargetVehicleDataPath == TargetPath
		&& Receipt.TargetDefinitionHash == TargetDefinitionHash)
	{
		return true;
	}

	// Legacy host-local token은 migration compatibility로만 허용합니다. same Target hash면 새 benchmark RunId에서도 USER PASS 의미를 유지합니다.
	return AcceptedDrivingRecipeId == Recipe->RecipeId
		&& AcceptedDrivingTargetDefinitionHash == TargetDefinitionHash;
}

// Current saved Target Definition을 USER Driving PASS persistent Recipe receipt + legacy local token으로 기록합니다.
bool FCFVehicleBuilderVM::AcceptCurrentUserDriving(FString& OutError)
{
	if (!bHasDrivingBenchmarkResult)
	{
		OutError = TEXT("USER Driving PASS 전에 current Technical Benchmark가 필요합니다.");
		return false;
	}
	if (!bUserTestDrivePreparedThisSession)
	{
		OutError = TEXT("USER Driving PASS 전에 Active PIE에 선택 차량을 적용하고 직접 주행해야 합니다.");
		return false;
	}

	// Current Recipe입니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	if (!Recipe || !Recipe->RecipeId.IsValid())
	{
		OutError = TEXT("USER Driving PASS를 binding할 current Recipe가 없습니다.");
		return false;
	}

	// Current Target identity입니다.
	FSoftObjectPath TargetPath;
	// Current Target exact hash입니다.
	FString TargetDefinitionHash;
	if (!BuildDrivingTargetIdentity(TargetPath, TargetDefinitionHash, OutError)
		|| TargetDefinitionHash != DrivingBenchmarkResult.ExpectedTargetDefinitionHash)
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("USER Driving PASS 대상과 benchmark Target hash가 일치하지 않습니다.");
		}
		return false;
	}

	// USER PASS는 current Target Definition에 대한 persistent non-semantic acceptance provenance입니다.
	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "AcceptBuilderDriving", "차량 Builder USER 주행 PASS"));
	Recipe->Modify();
	Recipe->BuilderDrivingAcceptanceReceipt.TargetVehicleDataPath = TargetPath;
	Recipe->BuilderDrivingAcceptanceReceipt.TargetDefinitionHash = TargetDefinitionHash;
	Recipe->BuilderDrivingAcceptanceReceipt.AcceptedBenchmarkRunId = DrivingBenchmarkResult.RunId;
	Recipe->MarkPackageDirty();
	Recipe->PostEditChange();

	// 구버전 local token도 당분간 함께 기록해 downgrade/legacy resume을 보존합니다.
	AcceptedDrivingRecipeId = Recipe->RecipeId;
	AcceptedDrivingTargetDefinitionHash = TargetDefinitionHash;
	AcceptedDrivingBenchmarkRunId = DrivingBenchmarkResult.RunId;
	SaveDrivingAcceptanceToken();
	RebuildStepStates();
	OutError.Reset();
	return true;
}

// Driving Test 단계의 existing VB-P0-08 technical benchmark + explicit USER Driving acceptance를 평가합니다.
void FCFVehicleBuilderVM::EvaluateDrivingTestStep()
{
	// Step 8은 Final Review current Complete 뒤에만 열립니다.
	FString PrerequisiteError;
	if (!AreDrivingTestPrerequisitesComplete(PrerequisiteError))
	{
		DrivingBenchmarkResult = FCFVehicleBuilderBenchmarkResult();
		bHasDrivingBenchmarkResult = false;
		DrivingBenchmarkStateError.Reset();
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			ECFVehicleBuilderStepState::Locked,
			PrerequisiteError,
			TEXT("Step 7 Final Review를 current Complete 상태로 만든 뒤 다시 확인하세요."),
			true);
		return;
	}

	// Current Step 8 Target identity입니다.
	FSoftObjectPath TargetPath;
	// Current Target exact hash입니다.
	FString TargetDefinitionHash;
	FString IdentityError;
	if (!BuildDrivingTargetIdentity(TargetPath, TargetDefinitionHash, IdentityError))
	{
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			ECFVehicleBuilderStepState::Blocked,
			IdentityError,
			TEXT("Current Recipe/Target/DefinitionHash binding을 복구한 뒤 다시 확인하세요."),
			true);
		return;
	}

	// Current Recipe/Target saved-state를 검사합니다.
	UCFVehicleRecipeData* Recipe = GetRecipe();
	// Current Target VehicleData입니다.
	UCFVehicleData* TargetVehicleData = Recipe ? Recipe->TargetVehicleData.LoadSynchronous() : nullptr;
	if (!Recipe || !TargetVehicleData)
	{
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			ECFVehicleBuilderStepState::Blocked,
			TEXT("Driving Test current Recipe/Target을 load할 수 없습니다."),
			TEXT("Current selection을 다시 읽고 Target binding을 복구하세요."),
			true);
		return;
	}

	// Benchmark/runtime truth는 saved Target VehicleData만 필요합니다. Recipe는 USER Driving receipt 같은 non-semantic metadata 때문에 Dirty일 수 있습니다.
	if (TargetVehicleData->GetOutermost()->IsDirty())
	{
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			bHasDrivingBenchmarkResult ? ECFVehicleBuilderStepState::Stale : ECFVehicleBuilderStepState::Ready,
			TEXT("Technical Benchmark와 USER Driving은 saved Target 기준입니다. Target VehicleData가 아직 Dirty입니다."),
			TEXT("Step 7 Apply 결과의 Target VehicleData를 직접 저장한 뒤 '기술 벤치마크 실행'을 사용하세요. Builder가 자동 저장하지 않습니다."),
			true);
		return;
	}

	// Existing runner result를 current Target path/hash에 fresh binding합니다.
	FString BenchmarkError;
	const bool bBenchmarkReadSucceeded = RefreshDrivingBenchmarkState(BenchmarkError);
	if (!bBenchmarkReadSucceeded)
	{
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			ECFVehicleBuilderStepState::Stale,
			DrivingBenchmarkStateError.IsEmpty() ? BenchmarkError : DrivingBenchmarkStateError,
			TEXT("Current saved Target으로 Technical Benchmark를 다시 실행하세요. Reference threshold는 임의 생성하지 않습니다."),
			true);
		return;
	}

	if (!bHasDrivingBenchmarkResult)
	{
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			ECFVehicleBuilderStepState::Ready,
			TEXT("Step 7은 Complete이며 saved Target으로 실행한 current Technical Benchmark가 아직 없습니다."),
			TEXT("기술 벤치마크를 실행한 뒤 결과를 확인하고, 실제 PIE에서 선택 차량을 직접 주행하세요."),
			true);
		return;
	}

	if (HasCurrentUserDrivingAcceptance())
	{
		SetStep(
			ECFVehicleBuilderStepId::DrivingTest,
			ECFVehicleBuilderStepState::Complete,
			FString::Printf(
				TEXT("Technical Benchmark는 current Target에 binding됐고 USER Driving PASS는 exact Target DefinitionHash에 persistent binding됐습니다. Current RunId=%s."),
				*DrivingBenchmarkResult.RunId),
			TEXT("VB-P0-09 신규 차량 E2E USER Acceptance를 닫을 수 있습니다."),
			true);
		return;
	}

	SetStep(
		ECFVehicleBuilderStepId::DrivingTest,
		ECFVehicleBuilderStepState::Ready,
		FString::Printf(
			TEXT("Technical Benchmark는 current Target에 binding됐습니다. USER Driving Feel 확인이 남았습니다. RunId=%s."),
			*DrivingBenchmarkResult.RunId),
		TEXT("플레이를 시작하고 '현재 PIE에 선택 차량 적용'으로 transient test-drive를 준비한 뒤 직접 주행하세요. 만족하면 'USER 주행 PASS'를 명시 승인하세요."),
		true);
}

// Stable StepId의 current presentation index를 찾습니다.
int32 FCFVehicleBuilderVM::FindStepIndexById(const ECFVehicleBuilderStepId StepId) const
{
	for (int32 StepIndex = 0; StepIndex < StepViews.Num(); ++StepIndex)
	{
		if (StepViews[StepIndex].StepId == StepId)
		{
			return StepIndex;
		}
	}
	return INDEX_NONE;
}

// Stable StepId로 Step 하나의 상태/설명을 갱신합니다.
void FCFVehicleBuilderVM::SetStep(
	const ECFVehicleBuilderStepId StepId,
	const ECFVehicleBuilderStepState State,
	const FString& Summary,
	const FString& Resolution,
	const bool bProviderConnected)
{
	// Stable StepId의 current presentation index입니다.
	const int32 StepIndex = FindStepIndexById(StepId);
	if (!StepViews.IsValidIndex(StepIndex))
	{
		return;
	}

	// 상태를 갱신할 current transient Step projection입니다.
	FCFVehicleBuilderStepView& Step = StepViews[StepIndex];
	Step.State = State;
	Step.Summary = FText::FromString(Summary);
	Step.Resolution = FText::FromString(Resolution);
	Step.bProviderConnected = bProviderConnected;

	// 디버깅 시 상태를 한눈에 읽기 위한 suffix입니다.
	Step.Summary = FText::FromString(FString::Printf(TEXT("[%s] %s"), StepStateText(State), *Step.Summary.ToString()));
}
