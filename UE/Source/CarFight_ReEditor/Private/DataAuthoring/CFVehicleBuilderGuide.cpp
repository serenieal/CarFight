// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBuilderGuide.cpp
// Version: v1.3.0
// Date: 2026-08-28
// Description: CF-FQ-040 Gameplay guidance + WSA-P0-04 mode-aware WheelVisual validation입니다.
// Scope: Durability, Defense, Destroyed FX, Hardpoint, Mount, DriveState, WheelVisual, Fitting mass의 current authored truth를 mutation 없이 설명합니다.
// Changelog:
// - v1.3.0: CF-FQ-043 VMG-P0-04 Step 6 Hardpoint/Mount authority를 CompanionMode에서 Recipe BuilderHardpointPlanMode로 분리. UseHardpoints에서 Standard 1:1, MountType/SizeLimit, EquipmentPreset CanUseOnMount를 검증하고 Legacy custom/multi-Mount는 보존.
// - v1.2.0: Socket/Manual mode에서 Legacy AutoScale clamp를 blocker에서 제외하고 Socket Scale USER authority summary/Reference geometry conflict를 추가.
// - v1.1.0: Final Review가 이미 계산한 fresh Resolve를 재사용할 수 있도록 projection helper를 분리해 동일 Resolver 반복 실행을 제거.
// - v1.0.0: 기존 Resolver/Asset/Target 계약을 재사용하는 8영역 R0 guidance와 Existing Vehicle baseline-preserving Hardpoint 안내를 최초 구현.
// Migration:
// - Hardpoint와 Destroyed FX Socket은 USER authority입니다. 이 파일은 StaticMesh Socket을 생성·이동·저장하지 않습니다.
// - Existing Vehicle Completion의 current Hardpoint LocalTransform은 current candidate와 충돌하지 않으면 그대로 인정하며 Socket 방식으로 강제 마이그레이션하지 않습니다.
// - Optional Defense/Destroyed FX/Fitting은 현재 Runtime fallback/명시적 None 계약을 존중하며 임의 Asset이나 수치를 만들지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "CFCombatFxData.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Engine/StaticMesh.h"

namespace CFVehicleBuilderGuidePrivate
{
	// R0 guidance operation의 stable identity입니다.
	const FName GuidanceOperationName(TEXT("ReadBuilderGameplayGuidance"));

	// current guidance 결과에 영역 하나를 추가하고 aggregate count를 갱신합니다.
	void AddGuidanceItem(
		FCFBuilderGameplayGuidanceResult& OutResult,
		const ECFBuilderGameplayArea Area,
		const ECFBuilderGuidanceState State,
		const FString& Summary,
		const FString& ResolutionText,
		const FString& RelatedFieldPath,
		const bool bUserActionRequired)
	{
		// 새로 추가할 영역별 guidance row입니다.
		FCFBuilderGameplayGuidanceItem& Item = OutResult.Items.AddDefaulted_GetRef();
		Item.Area = Area;
		Item.State = State;
		Item.Summary = Summary;
		Item.ResolutionText = ResolutionText;
		Item.RelatedFieldPath = RelatedFieldPath;
		Item.bUserActionRequired = bUserActionRequired;

		if (State == ECFBuilderGuidanceState::NeedsReview)
		{
			++OutResult.NeedsReviewCount;
		}
		else if (State == ECFBuilderGuidanceState::Blocked)
		{
			++OutResult.BlockedCount;
		}
	}

	// Soft object path가 기대한 UObject class로 실제 resolve/load 가능한지 read-only로 확인합니다.
	bool IsAssetLoadableAs(const FSoftObjectPath& AssetPath, const UClass* ExpectedClass, FString& OutFailureReason)
	{
		if (!AssetPath.IsValid() || !ExpectedClass)
		{
			OutFailureReason = TEXT("Asset path 또는 expected class가 유효하지 않습니다.");
			return false;
		}

		// 이미 메모리에 존재하거나 path에서 read-only load한 object입니다.
		UObject* LoadedObject = AssetPath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = AssetPath.TryLoad();
		}

		if (!LoadedObject || !LoadedObject->IsA(ExpectedClass))
		{
			OutFailureReason = FString::Printf(
				TEXT("Asset을 기대 타입으로 읽을 수 없습니다: %s"),
				*AssetPath.ToString());
			return false;
		}

		OutFailureReason.Reset();
		return true;
	}

	// Recipe/explicit Target identity에서 current VehicleData UObject를 read-only로 얻습니다.
	UCFVehicleData* ResolveCurrentTarget(const FCFBuilderGameplayGuidanceRequest& Request)
	{
		if (Request.ReadRequest.TargetVehicleData)
		{
			return Request.ReadRequest.TargetVehicleData;
		}

		if (!Request.ReadRequest.Recipe)
		{
			return nullptr;
		}

		// Recipe가 binding한 current Target VehicleData입니다.
		UCFVehicleData* TargetVehicleData = Request.ReadRequest.Recipe->TargetVehicleData.Get();
		if (!TargetVehicleData)
		{
			TargetVehicleData = Request.ReadRequest.Recipe->TargetVehicleData.LoadSynchronous();
		}
		return TargetVehicleData;
	}

	// Recipe Chassis soft reference에서 current StaticMesh를 read-only로 얻습니다.
	UStaticMesh* ResolveChassisMesh(const FCFVehicleRecipeSnapshot& RecipeSnapshot)
	{
		// Recipe가 현재 선택한 Chassis StaticMesh path입니다.
		const FSoftObjectPath ChassisPath = RecipeSnapshot.AssetIntent.ChassisMesh.ToSoftObjectPath();
		if (!ChassisPath.IsValid())
		{
			return nullptr;
		}

		// 이미 메모리에 존재하거나 path에서 read-only load한 Chassis object입니다.
		UObject* LoadedObject = ChassisPath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = ChassisPath.TryLoad();
		}
		return Cast<UStaticMesh>(LoadedObject);
	}

	// Hardpoint intent에 SocketName이 없을 때 USER에게 제안할 표준 HP_<LocationSlotId> 이름을 만듭니다.
	FName BuildSuggestedHardpointSocketName(const FName LocationSlotId)
	{
		if (LocationSlotId.IsNone())
		{
			return NAME_None;
		}

		// LocationSlotId를 포함한 표준 수동 Socket 이름입니다.
		const FString SuggestedName = FString::Printf(TEXT("HP_%s"), *LocationSlotId.ToString());
		return FName(*SuggestedName);
	}

	// current Target의 stable LocationSlotId와 일치하는 HardpointSlot을 찾습니다.
	const FCFVehicleHardpointSlot* FindCurrentHardpointSlot(const UCFVehicleData* TargetVehicleData, const FName LocationSlotId)
	{
		if (!TargetVehicleData || LocationSlotId.IsNone())
		{
			return nullptr;
		}

		return TargetVehicleData->HardpointSlots.FindByPredicate([LocationSlotId](const FCFVehicleHardpointSlot& Slot)
		{
			return Slot.LocationSlotId == LocationSlotId;
		});
	}

	// Resolver pending diff에 canonical field prefix가 하나라도 존재하는지 확인합니다.
	bool HasDiffPrefix(const TArray<FCFVehicleFieldDiff>& FieldDiff, const FString& CanonicalPrefix)
	{
		return FieldDiff.ContainsByPredicate([&CanonicalPrefix](const FCFVehicleFieldDiff& Diff)
		{
			return Diff.FieldPath.ToCanonicalString().StartsWith(CanonicalPrefix);
		});
	}

	// Resolver pending diff에 exact canonical field path가 존재하는지 확인합니다.
	bool HasExactDiff(const TArray<FCFVehicleFieldDiff>& FieldDiff, const FString& CanonicalFieldPath)
	{
		return FieldDiff.ContainsByPredicate([&CanonicalFieldPath](const FCFVehicleFieldDiff& Diff)
		{
			return Diff.FieldPath.ToCanonicalString() == CanonicalFieldPath;
		});
	}

	// Existing Vehicle의 Hardpoint stored transform이 current candidate와 충돌하는 diff를 갖는지 확인합니다.
	bool HasHardpointTransformDiff(const TArray<FCFVehicleFieldDiff>& FieldDiff, const FName LocationSlotId)
	{
		if (LocationSlotId.IsNone())
		{
			return true;
		}

		// Stable-ID Hardpoint LocalLocation canonical path입니다.
		const FString LocationPath = FString::Printf(
			TEXT("HardpointSlots[LocationSlotId=%s].LocalLocation"),
			*LocationSlotId.ToString());

		// Stable-ID Hardpoint LocalRotation canonical path입니다.
		const FString RotationPath = FString::Printf(
			TEXT("HardpointSlots[LocationSlotId=%s].LocalRotation"),
			*LocationSlotId.ToString());

		return HasExactDiff(FieldDiff, LocationPath) || HasExactDiff(FieldDiff, RotationPath);
	}

	// Diff 하나가 VB-P0-06 Gameplay Setup owner 영역인지 분류합니다.
	bool IsGameplayDiff(const FCFVehicleFieldDiff& Diff)
	{
		// Stable Field Path의 canonical 문자열입니다.
		const FString Path = Diff.FieldPath.ToCanonicalString();
		return Path.StartsWith(TEXT("VehicleDurabilityConfig."))
			|| Path == TEXT("DefaultDefenseData")
			|| Path == TEXT("DefaultDestroyedFxData")
			|| Path == TEXT("DestroyedFxSocketName")
			|| Path.StartsWith(TEXT("HardpointSlots"))
			|| Path.StartsWith(TEXT("MountProfiles"))
			|| Path == TEXT("BaseVehicleMassKg")
			|| Path == TEXT("MaximumGrossMassKg")
			|| Path.StartsWith(TEXT("WheelVisualConfig."))
			|| Path.StartsWith(TEXT("DriveStateConfig."));
	}

	// Chassis StaticMesh에 exact Socket 이름이 존재하는지 확인합니다.
	bool HasChassisSocket(const UStaticMesh* ChassisMesh, const FName SocketName)
	{
		return ChassisMesh && !SocketName.IsNone() && ChassisMesh->FindSocket(SocketName) != nullptr;
	}

	// USER-owned Socket 하나의 현재 상태와 작업 안내를 결과에 추가합니다.
	void AddSocketGuidance(
		FCFBuilderGameplayGuidanceResult& OutResult,
		const ECFBuilderGameplayArea Area,
		const FName SemanticId,
		const FName SocketName,
		const bool bFoundOnChassis,
		const bool bExistingStoredTransformAccepted,
		const FString& Instruction)
	{
		// 새로 추가할 manual Socket guidance row입니다.
		FCFBuilderManualSocketGuidance& SocketGuidance = OutResult.SocketGuidance.AddDefaulted_GetRef();
		SocketGuidance.Area = Area;
		SocketGuidance.SemanticId = SemanticId;
		SocketGuidance.SocketName = SocketName;
		SocketGuidance.bFoundOnChassis = bFoundOnChassis;
		SocketGuidance.bExistingStoredTransformAccepted = bExistingStoredTransformAccepted;
		SocketGuidance.Instruction = Instruction;
	}

	// VehicleBase Profile이 WheelVisual policy를 완전히 제공할 수 있는지 검사합니다.
	bool IsWheelVisualProfileComplete(const FCFVehicleBaseProfileData& BaseData, const ECFWheelVisualIntentMode WheelVisualMode, FString& OutFailureReason)
	{
		if (BaseData.ExpectedWheelCount <= 0)
		{
			OutFailureReason = TEXT("VehicleBase Profile의 ExpectedWheelCount가 1 이상이어야 합니다.");
			return false;
		}

		if (BaseData.FrontWheelCountForSteering < 0
			|| BaseData.FrontWheelCountForSteering > BaseData.ExpectedWheelCount)
		{
			OutFailureReason = TEXT("VehicleBase Profile의 FrontWheelCountForSteering이 ExpectedWheelCount 범위를 벗어납니다.");
			return false;
		}

		if (WheelVisualMode == ECFWheelVisualIntentMode::SocketScaleFromChassis && BaseData.bUseReferenceWheelGeometry)
		{
			OutFailureReason = TEXT("Socket Scale mode에서는 VehicleBase Profile의 Reference wheel geometry를 Wheel Size authority로 사용할 수 없습니다. bUseReferenceWheelGeometry를 끄고 Reference 값은 sanity reference로만 유지하세요.");
			return false;
		}

		const bool bLegacyAutoScaleEffective = WheelVisualMode == ECFWheelVisualIntentMode::AutoScaleToPhysicsRadius
			|| (WheelVisualMode == ECFWheelVisualIntentMode::UseProfilePolicy && BaseData.bAutoScaleWheelMeshToRadius);
		if (bLegacyAutoScaleEffective)
		{
			// 자동 스케일 최소 제한값입니다.
			const float ClampMin = BaseData.WheelMeshScaleClampMin;
			// 자동 스케일 최대 제한값입니다.
			const float ClampMax = BaseData.WheelMeshScaleClampMax;
			if (!FMath::IsFinite(ClampMin)
				|| !FMath::IsFinite(ClampMax)
				|| ClampMin <= 0.0f
				|| ClampMax <= 0.0f
				|| ClampMin > ClampMax)
			{
				OutFailureReason = TEXT("VehicleBase Profile의 WheelVisual auto-scale clamp가 유효하지 않습니다.");
				return false;
			}
		}

		OutFailureReason.Reset();
		return true;
	}
}

// current Recipe/Target/Resolver/Asset truth에서 Gameplay Setup completeness와 USER 수동 작업 안내를 mutation0로 반환합니다.
bool FCFVehicleAuthoringService::ReadBuilderGameplayGuidance(
	const FCFBuilderGameplayGuidanceRequest& Request,
	FCFBuilderGameplayGuidanceResult& OutResult)
{
	// Existing shared Core에서 fresh resolve/asset/target snapshot을 얻는 R0 결과입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request.ReadRequest, ResolveRead))
	{
		OutResult = FCFBuilderGameplayGuidanceResult();
		OutResult.Mode = Request.Mode;
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = CFVehicleBuilderGuidePrivate::GuidanceOperationName;
		OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
		return false;
	}
	return BuildBuilderGameplayGuidanceFromResolve(Request, ResolveRead, OutResult);
}

// Final Review가 이미 확보한 one fresh Resolve result에서 Gameplay projection만 구성합니다.
bool FCFVehicleAuthoringService::BuildBuilderGameplayGuidanceFromResolve(
	const FCFBuilderGameplayGuidanceRequest& Request,
	const FCFVehicleResolveReadResult& ResolveRead,
	FCFBuilderGameplayGuidanceResult& OutResult)
{
	OutResult = FCFBuilderGameplayGuidanceResult();
	OutResult.Mode = Request.Mode;
	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = CFVehicleBuilderGuidePrivate::GuidanceOperationName;
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;

	// Fresh immutable Recipe semantic snapshot입니다.
	const FCFVehicleRecipeSnapshot& Recipe = ResolveRead.ResolveRequest.Recipe;
	// Fresh immutable Profile snapshot set입니다.
	const FCFVehicleProfileSnapshotSet& Profiles = ResolveRead.ResolveRequest.Profiles;
	// Fresh immutable Asset snapshot입니다.
	const FCFVehicleAssetSnapshot& Assets = ResolveRead.ResolveRequest.Assets;
	// Fresh Pure Resolver 결과입니다.
	const FCFVehicleResolveResult& ResolveResult = ResolveRead.ResolveResult;
	// Existing Vehicle Completion의 baseline readback에 사용할 current Target입니다.
	UCFVehicleData* CurrentTarget = CFVehicleBuilderGuidePrivate::ResolveCurrentTarget(Request);
	// Destroyed FX Socket과 suggested Hardpoint Socket을 read-only 확인할 current Chassis입니다.
	UStaticMesh* ChassisMesh = CFVehicleBuilderGuidePrivate::ResolveChassisMesh(Recipe);

	// Durability source mode가 명시 값인지 여부입니다.
	const bool bExplicitDurability = Recipe.DurabilityIntent.MaxHealthMode == ECFAuthoringInputMode::ExplicitValue;
	// current semantic source가 만드는 최대 내구도 값입니다.
	const float EffectiveMaxHealth = bExplicitDurability
		? Recipe.DurabilityIntent.ExplicitMaxHealth
		: Profiles.BaseData.MaxHealth;
	if (!bExplicitDurability && !Profiles.BaseSource.IsPresent())
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::Durability,
			ECFBuilderGuidanceState::Blocked,
			TEXT("내구도는 VehicleBase Profile을 사용하도록 되어 있지만 Profile source가 없습니다."),
			TEXT("Builder-private VehicleBase Profile binding을 복구한 뒤 다시 새로고침하세요. Runtime fallback 100으로 Authoring 누락을 숨기지 않습니다."),
			TEXT("DurabilityIntent"),
			false);
	}
	else if (!FMath::IsFinite(EffectiveMaxHealth) || EffectiveMaxHealth <= 0.0f)
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::Durability,
			ECFBuilderGuidanceState::Blocked,
			TEXT("현재 Authoring source의 최대 내구도가 유효한 양수가 아닙니다."),
			TEXT("실제 차량/게임 밸런스 기준으로 MaxHealth를 명시하세요. Builder는 임의 체력값을 생성하지 않습니다."),
			TEXT("VehicleDurabilityConfig.MaxHealth"),
			true);
	}
	else
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::Durability,
			ECFBuilderGuidanceState::Complete,
			FString::Printf(TEXT("최대 내구도 %.2f가 current semantic source에서 결정됩니다."), EffectiveMaxHealth),
			TEXT("추가 작업이 필요하지 않습니다."),
			TEXT("VehicleDurabilityConfig.MaxHealth"),
			false);
	}

	// Defense intent가 실제로 선택한 optional asset path입니다.
	FSoftObjectPath DefensePath;
	// Defense가 의도적으로 없는 상태인지 여부입니다.
	bool bDefenseExplicitNone = false;
	// Defense Profile source 자체가 누락됐는지 여부입니다.
	bool bDefenseProfileMissing = false;
	if (Recipe.DefaultDataIntent.DefenseMode == ECFAssetIntentMode::ExplicitAsset)
	{
		DefensePath = Recipe.DefaultDataIntent.DefaultDefenseData.ToSoftObjectPath();
	}
	else if (Recipe.DefaultDataIntent.DefenseMode == ECFAssetIntentMode::ExplicitNone)
	{
		bDefenseExplicitNone = true;
	}
	else if (Profiles.BaseSource.IsPresent())
	{
		DefensePath = Profiles.BaseData.DefaultDefenseData.ToSoftObjectPath();
	}
	else
	{
		bDefenseProfileMissing = true;
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::Defense,
			ECFBuilderGuidanceState::Blocked,
			TEXT("기본 방어는 VehicleBase Profile을 사용하도록 되어 있지만 Profile source가 없습니다."),
			TEXT("VehicleBase Profile binding을 복구하거나 Defense 입력 방식을 명시 자산/명시적으로 없음으로 선택하세요."),
			TEXT("DefaultDataIntent.DefenseMode"),
			false);
	}

	if (!bDefenseProfileMissing)
	{
		if (bDefenseExplicitNone || !DefensePath.IsValid())
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::Defense,
				ECFBuilderGuidanceState::Optional,
				bDefenseExplicitNone
					? TEXT("방어 데이터가 명시적으로 없음으로 설정되어 있습니다.")
					: TEXT("기본 방어 DataAsset이 없습니다. 현재 Runtime의 직접 내구도 피해 fallback이 유효합니다."),
				TEXT("쉴드/방향 장갑이 필요한 차량에서만 기존 VehicleDefenseData를 명시적으로 선택하세요. Builder가 방어 Asset을 임의 생성하지 않습니다."),
				TEXT("DefaultDefenseData"),
				false);
		}
		else
		{
			// Defense asset type/readability failure 이유입니다.
			FString DefenseFailure;
			if (!CFVehicleBuilderGuidePrivate::IsAssetLoadableAs(DefensePath, UCFVehicleDefenseData::StaticClass(), DefenseFailure))
			{
				CFVehicleBuilderGuidePrivate::AddGuidanceItem(
					OutResult,
					ECFBuilderGameplayArea::Defense,
					ECFBuilderGuidanceState::Blocked,
					DefenseFailure,
					TEXT("기존 VehicleDefenseData Asset을 다시 선택하거나 명시적으로 없음 정책을 선택하세요."),
					TEXT("DefaultDefenseData"),
					true);
			}
			else
			{
				CFVehicleBuilderGuidePrivate::AddGuidanceItem(
					OutResult,
					ECFBuilderGameplayArea::Defense,
					ECFBuilderGuidanceState::Complete,
					FString::Printf(TEXT("기본 방어 DataAsset을 사용합니다: %s"), *DefensePath.ToString()),
					TEXT("추가 작업이 필요하지 않습니다."),
					TEXT("DefaultDefenseData"),
					false);
			}
		}
	}

	// Destroyed FX intent가 실제로 선택한 optional asset path입니다.
	FSoftObjectPath DestroyedFxPath;
	// Destroyed FX가 의도적으로 없는 상태인지 여부입니다.
	bool bDestroyedFxExplicitNone = false;
	// Destroyed FX Profile source 자체가 누락됐는지 여부입니다.
	bool bDestroyedFxProfileMissing = false;
	if (Recipe.DefaultDataIntent.DestroyedFxMode == ECFAssetIntentMode::ExplicitAsset)
	{
		DestroyedFxPath = Recipe.DefaultDataIntent.DefaultDestroyedFxData.ToSoftObjectPath();
	}
	else if (Recipe.DefaultDataIntent.DestroyedFxMode == ECFAssetIntentMode::ExplicitNone)
	{
		bDestroyedFxExplicitNone = true;
	}
	else if (Profiles.BaseSource.IsPresent())
	{
		DestroyedFxPath = Profiles.BaseData.DefaultDestroyedFxData.ToSoftObjectPath();
	}
	else
	{
		bDestroyedFxProfileMissing = true;
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::DestroyedFx,
			ECFBuilderGuidanceState::Blocked,
			TEXT("파괴 FX는 VehicleBase Profile을 사용하도록 되어 있지만 Profile source가 없습니다."),
			TEXT("VehicleBase Profile binding을 복구하거나 파괴 FX 입력 방식을 명시 자산/명시적으로 없음으로 선택하세요."),
			TEXT("DefaultDataIntent.DestroyedFxMode"),
			false);
	}

	if (!bDestroyedFxProfileMissing)
	{
		if (bDestroyedFxExplicitNone || !DestroyedFxPath.IsValid())
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::DestroyedFx,
				ECFBuilderGuidanceState::Optional,
				bDestroyedFxExplicitNone
					? TEXT("파괴 FX가 명시적으로 없음으로 설정되어 있습니다.")
					: TEXT("기본 파괴 FX DataAsset이 없습니다. 차량 파괴 판정 자체는 계속 동작합니다."),
				TEXT("파괴 연출이 필요할 때만 기존 CombatFxData를 선택하세요. Builder가 FX Asset을 임의 생성하지 않습니다."),
				TEXT("DefaultDestroyedFxData"),
				false);
		}
		else
		{
			// Destroyed FX asset type/readability failure 이유입니다.
			FString DestroyedFxFailure;
			if (!CFVehicleBuilderGuidePrivate::IsAssetLoadableAs(DestroyedFxPath, UCFCombatFxData::StaticClass(), DestroyedFxFailure))
			{
				CFVehicleBuilderGuidePrivate::AddGuidanceItem(
					OutResult,
					ECFBuilderGameplayArea::DestroyedFx,
					ECFBuilderGuidanceState::Blocked,
					DestroyedFxFailure,
					TEXT("기존 CombatFxData Asset을 다시 선택하거나 명시적으로 없음 정책을 선택하세요."),
					TEXT("DefaultDestroyedFxData"),
					true);
			}
			else
			{
				// Recipe가 현재 의도한 Destroyed FX Chassis Socket 이름입니다.
				const FName DestroyedFxSocketName = Recipe.DefaultDataIntent.DestroyedFxSocketName.IsNone()
					? FName(TEXT("FX_Destroyed"))
					: Recipe.DefaultDataIntent.DestroyedFxSocketName;
				// Chassis에 exact Destroyed FX Socket이 존재하는지 여부입니다.
				const bool bDestroyedFxSocketFound = CFVehicleBuilderGuidePrivate::HasChassisSocket(ChassisMesh, DestroyedFxSocketName);
				CFVehicleBuilderGuidePrivate::AddGuidanceItem(
					OutResult,
					ECFBuilderGameplayArea::DestroyedFx,
					ECFBuilderGuidanceState::Complete,
					FString::Printf(TEXT("기본 파괴 FX DataAsset을 사용합니다: %s"), *DestroyedFxPath.ToString()),
					bDestroyedFxSocketFound
						? TEXT("파괴 FX Socket도 현재 Chassis에서 확인됐습니다.")
						: TEXT("Socket이 없어도 Runtime은 차체 Bounds 중심 fallback을 사용합니다. 정확한 연출 위치가 필요할 때만 USER가 Socket을 직접 배치하세요."),
					TEXT("DefaultDestroyedFxData / DestroyedFxSocketName"),
					false);

				CFVehicleBuilderGuidePrivate::AddSocketGuidance(
					OutResult,
					ECFBuilderGameplayArea::DestroyedFx,
					TEXT("FX_Destroyed"),
					DestroyedFxSocketName,
					bDestroyedFxSocketFound,
					false,
					bDestroyedFxSocketFound
						? TEXT("Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 USER가 배치한 현재 위치를 그대로 사용합니다. Builder는 이 Socket을 이동하지 않습니다.")
						: FString::Printf(TEXT("정확한 파괴 FX 위치가 필요하면 Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 %s를 USER가 직접 추가·배치하세요. Builder는 자동배치하지 않습니다."), *DestroyedFxSocketName.ToString()));
			}
		}
	}

	// Hardpoint stable ID 중복을 검출하는 집합입니다.
	TSet<FName> SeenHardpointIds;
	// Hardpoint intent 자체에 blocker가 있는지 여부입니다.
	bool bHardpointBlocked = false;
	// Hardpoint intent 중 USER review/action이 필요한 항목 수입니다.
	int32 HardpointNeedsReviewCount = 0;
	if (Recipe.HardpointIntents.IsEmpty())
	{
		if (Request.HardpointPlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible
			&& CurrentTarget
			&& !CurrentTarget->HardpointSlots.IsEmpty())
		{
			for (const FCFVehicleHardpointSlot& ExistingSlot : CurrentTarget->HardpointSlots)
			{
				// Existing slot의 authored Socket 또는 표준 제안 이름입니다.
				const FName ExistingSocketName = ExistingSlot.SocketName.IsNone()
					? CFVehicleBuilderGuidePrivate::BuildSuggestedHardpointSocketName(ExistingSlot.LocationSlotId)
					: ExistingSlot.SocketName;
				// Existing slot의 Chassis Socket 존재 여부입니다.
				const bool bExistingSocketFound = CFVehicleBuilderGuidePrivate::HasChassisSocket(ChassisMesh, ExistingSocketName);
				CFVehicleBuilderGuidePrivate::AddSocketGuidance(
					OutResult,
					ECFBuilderGameplayArea::Hardpoints,
					ExistingSlot.LocationSlotId,
					ExistingSocketName,
					bExistingSocketFound,
					true,
					bExistingSocketFound
						? TEXT("기존 차량의 stored LocalTransform을 보존하면서 USER가 배치한 Socket도 참고할 수 있습니다. Builder는 위치를 자동 변경하지 않습니다.")
						: TEXT("Existing Vehicle Completion은 현재 VehicleData의 stored LocalTransform을 그대로 보존합니다. 다시 캡처할 필요가 생길 때만 USER가 Socket을 직접 추가·배치하세요."));
			}

			// Existing Hardpoint collection이 current authoring candidate에서 실제 변경되는지 여부입니다.
			const bool bExistingHardpointDiff = CFVehicleBuilderGuidePrivate::HasDiffPrefix(ResolveResult.FieldDiff, TEXT("HardpointSlots"));
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::Hardpoints,
				bExistingHardpointDiff ? ECFBuilderGuidanceState::NeedsReview : ECFBuilderGuidanceState::Complete,
				bExistingHardpointDiff
					? TEXT("Existing Vehicle의 HardpointSlots가 current authoring candidate와 다릅니다.")
					: TEXT("Recipe에 새 Hardpoint intent는 없지만 Existing Vehicle의 stored HardpointSlots를 current authoring state가 그대로 보존합니다."),
				bExistingHardpointDiff
					? TEXT("기존 Hardpoint를 삭제·재배치하지 말고 Legacy/Recipe ownership을 먼저 review하세요. P0-06은 위치를 자동 수정하지 않습니다.")
					: TEXT("기존 배치를 유지합니다. Builder-managed recapture가 필요해질 때만 stable ID와 Socket binding을 추가하세요."),
				TEXT("HardpointSlots"),
				bExistingHardpointDiff);
		}
		else
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::Hardpoints,
				ECFBuilderGuidanceState::Optional,
				TEXT("하드포인트가 선언되지 않았습니다. current contract에서 빈 배열은 오류가 아닙니다."),
				TEXT("전투 장비 위치가 필요한 차량에서만 Top_01 / Front_01 같은 stable slot을 추가하고 실제 Socket 위치는 USER가 직접 배치하세요."),
				TEXT("HardpointIntents"),
				false);
		}
	}
	else
	{
		for (const FCFHardpointIntent& HardpointIntent : Recipe.HardpointIntents)
		{
			if (HardpointIntent.LocationSlotId.IsNone() || SeenHardpointIds.Contains(HardpointIntent.LocationSlotId))
			{
				bHardpointBlocked = true;
				continue;
			}
			SeenHardpointIds.Add(HardpointIntent.LocationSlotId);

			// Recipe가 실제 binding한 Socket 이름입니다.
			const FName AuthoredSocketName = HardpointIntent.SocketName;
			// SocketName이 비었을 때 USER에게 제안할 exact 표준 이름입니다.
			const FName GuidanceSocketName = AuthoredSocketName.IsNone()
				? CFVehicleBuilderGuidePrivate::BuildSuggestedHardpointSocketName(HardpointIntent.LocationSlotId)
				: AuthoredSocketName;
			// Recipe Asset snapshot에서 authored Socket을 찾았는지 여부입니다.
			const FCFVehicleSocketSnapshot* SocketSnapshot = AuthoredSocketName.IsNone()
				? nullptr
				: Assets.FindChassisSocket(AuthoredSocketName);
			// Authored Socket이 current Chassis에 실제 존재하는지 여부입니다.
			const bool bAuthoredSocketFound = SocketSnapshot && SocketSnapshot->bFound;
			// Existing Target에 같은 stable Hardpoint가 이미 존재하는지 여부입니다.
			const FCFVehicleHardpointSlot* ExistingSlot = CFVehicleBuilderGuidePrivate::FindCurrentHardpointSlot(CurrentTarget, HardpointIntent.LocationSlotId);
			// Existing stored transform과 current candidate가 충돌하지 않아 보존 가능한지 여부입니다.
			const bool bExistingTransformAccepted = Request.HardpointPlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible
				&& ExistingSlot
				&& !CFVehicleBuilderGuidePrivate::HasHardpointTransformDiff(ResolveResult.FieldDiff, HardpointIntent.LocationSlotId);

			if (AuthoredSocketName.IsNone())
			{
				if (!bExistingTransformAccepted)
				{
					++HardpointNeedsReviewCount;
				}
				CFVehicleBuilderGuidePrivate::AddSocketGuidance(
					OutResult,
					ECFBuilderGameplayArea::Hardpoints,
					HardpointIntent.LocationSlotId,
					GuidanceSocketName,
					CFVehicleBuilderGuidePrivate::HasChassisSocket(ChassisMesh, GuidanceSocketName),
					bExistingTransformAccepted,
					bExistingTransformAccepted
						? FString::Printf(TEXT("Existing Vehicle의 %s stored LocalTransform은 그대로 보존할 수 있습니다. 향후 재캡처가 필요하면 Recipe SocketName을 %s로 연결하고 USER가 위치를 직접 관리하세요."), *HardpointIntent.LocationSlotId.ToString(), *GuidanceSocketName.ToString())
						: FString::Printf(TEXT("Recipe SocketName을 %s로 연결하고 Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 USER가 직접 위치를 배치하세요. Builder는 자동배치하지 않습니다."), *GuidanceSocketName.ToString()));
			}
			else if (!bAuthoredSocketFound)
			{
				if (!bExistingTransformAccepted)
				{
					++HardpointNeedsReviewCount;
				}
				CFVehicleBuilderGuidePrivate::AddSocketGuidance(
					OutResult,
					ECFBuilderGameplayArea::Hardpoints,
					HardpointIntent.LocationSlotId,
					AuthoredSocketName,
					false,
					bExistingTransformAccepted,
					bExistingTransformAccepted
						? TEXT("Existing Vehicle의 stored LocalTransform을 보존합니다. 이 Socket은 재캡처를 원할 때만 USER가 직접 추가·배치하면 됩니다.")
						: FString::Printf(TEXT("Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 %s를 USER가 직접 추가·배치하세요. Builder는 자동배치하지 않습니다."), *AuthoredSocketName.ToString()));
			}
			else
			{
				CFVehicleBuilderGuidePrivate::AddSocketGuidance(
					OutResult,
					ECFBuilderGameplayArea::Hardpoints,
					HardpointIntent.LocationSlotId,
					AuthoredSocketName,
					true,
					false,
					TEXT("Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 USER가 배치한 현재 위치가 authority입니다. Builder는 이 Socket을 이동하거나 자동 생성하지 않습니다."));
			}
		}

		if (bHardpointBlocked)
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::Hardpoints,
				ECFBuilderGuidanceState::Blocked,
				TEXT("Hardpoint LocationSlotId가 비어 있거나 중복되어 stable identity가 성립하지 않습니다."),
				TEXT("각 위치에 Top_01, Front_01처럼 고유한 LocationSlotId를 지정하세요. 실제 위치는 Chassis Socket을 USER가 직접 배치합니다."),
				TEXT("HardpointIntents"),
				true);
		}
		else if (HardpointNeedsReviewCount > 0)
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::Hardpoints,
				ECFBuilderGuidanceState::NeedsReview,
				FString::Printf(TEXT("하드포인트 %d개가 USER Socket binding/배치를 확인해야 합니다."), HardpointNeedsReviewCount),
				TEXT("각 guidance에 표시된 Socket 이름을 확인하고 필요한 경우 Static Mesh 에디터에서 직접 추가·배치하세요. Builder는 위치를 자동 계산·배치하지 않습니다."),
				TEXT("HardpointIntents"),
				true);
		}
		else
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::Hardpoints,
				ECFBuilderGuidanceState::Complete,
				TEXT("모든 Builder-managed Hardpoint가 stable ID와 현재 사용 가능한 위치 authority를 가집니다."),
				TEXT("추가 자동배치는 수행하지 않습니다."),
				TEXT("HardpointIntents"),
				false);
		}
	}

	// Step 6 Mount completion authority는 Companion lifecycle과 분리된 persistent Hardpoint Plan Mode입니다.
	const ECFBuilderHardpointPlanMode HardpointPlanMode = Request.HardpointPlanMode;

	if (HardpointPlanMode == ECFBuilderHardpointPlanMode::Unspecified)
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::MountProfiles,
			ECFBuilderGuidanceState::NeedsReview,
			TEXT("Hardpoint Plan이 아직 미결정이라 Standard Mount 규칙을 확정할 수 없습니다."),
			TEXT("Step 3에서 '장착점 없음' 또는 '장착 위치 사용'을 먼저 명시적으로 선택하세요. 기존 intent가 있어도 Builder가 silent mode 전환하지 않습니다."),
			TEXT("BuilderHardpointPlanMode"),
			true);
	}
	else if (HardpointPlanMode == ECFBuilderHardpointPlanMode::NoHardpoints)
	{
		const bool bSemanticArraysEmpty = Recipe.HardpointIntents.IsEmpty() && Recipe.MountIntents.IsEmpty();
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::MountProfiles,
			bSemanticArraysEmpty ? ECFBuilderGuidanceState::Complete : ECFBuilderGuidanceState::Blocked,
			bSemanticArraysEmpty
				? TEXT("장착점 없음이 명시되어 있어 MountProfile 0개가 intentional completion입니다.")
				: FString::Printf(TEXT("NoHardpoints와 semantic data가 충돌합니다. Hardpoint=%d / Mount=%d."), Recipe.HardpointIntents.Num(), Recipe.MountIntents.Num()),
			bSemanticArraysEmpty
				? TEXT("추가 Mount 작업이 필요하지 않습니다.")
				: TEXT("Mount를 먼저 제거하고 Hardpoint를 제거한 뒤 NoHardpoints 상태를 다시 확인하세요. 자동 cascade 삭제는 하지 않습니다."),
			TEXT("BuilderHardpointPlanMode / MountIntents"),
			!bSemanticArraysEmpty);
	}
	else if (HardpointPlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible)
	{
		// LegacyCompatible에서는 Existing/custom/multi-Mount를 새 Standard 1:1 규칙으로 강제 migration하지 않습니다.
		if (Recipe.MountIntents.IsEmpty() && CurrentTarget && !CurrentTarget->MountProfiles.IsEmpty())
		{
			const bool bExistingMountDiff = CFVehicleBuilderGuidePrivate::HasDiffPrefix(ResolveResult.FieldDiff, TEXT("MountProfiles"));
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::MountProfiles,
				bExistingMountDiff ? ECFBuilderGuidanceState::NeedsReview : ECFBuilderGuidanceState::Complete,
				bExistingMountDiff
					? TEXT("LegacyCompatible Existing Vehicle의 MountProfiles가 current authoring candidate와 다릅니다.")
					: TEXT("Recipe에 새 Mount intent가 없어도 LegacyCompatible은 Existing Vehicle의 MountProfiles를 그대로 보존합니다."),
				bExistingMountDiff
					? TEXT("기존 multi-Mount/custom ID를 자동 정리하지 말고 ownership/diff를 먼저 review하세요.")
					: TEXT("기존 장착 규칙을 유지합니다. Standard 1:1 편집은 '장착 위치 사용'으로 명시 전환한 뒤에만 적용합니다."),
				TEXT("MountProfiles"),
				bExistingMountDiff);
		}
		else
		{
			TSet<FName> LegacySeenMountIds;
			TSet<FName> LegacyKnownHardpointIds;
			for (const FCFHardpointIntent& HardpointIntent : Recipe.HardpointIntents)
			{
				if (!HardpointIntent.LocationSlotId.IsNone())
				{
					LegacyKnownHardpointIds.Add(HardpointIntent.LocationSlotId);
				}
			}
			if (CurrentTarget)
			{
				for (const FCFVehicleHardpointSlot& ExistingSlot : CurrentTarget->HardpointSlots)
				{
					if (!ExistingSlot.LocationSlotId.IsNone())
					{
						LegacyKnownHardpointIds.Add(ExistingSlot.LocationSlotId);
					}
				}
			}

			bool bLegacyMountBlocked = false;
			for (const FCFMountIntent& MountIntent : Recipe.MountIntents)
			{
				if (MountIntent.MountProfileId.IsNone()
					|| LegacySeenMountIds.Contains(MountIntent.MountProfileId)
					|| MountIntent.LocationSlotRef.IsNone()
					|| !LegacyKnownHardpointIds.Contains(MountIntent.LocationSlotRef))
				{
					bLegacyMountBlocked = true;
					continue;
				}
				LegacySeenMountIds.Add(MountIntent.MountProfileId);
			}

			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::MountProfiles,
				bLegacyMountBlocked ? ECFBuilderGuidanceState::Blocked : ECFBuilderGuidanceState::Complete,
				bLegacyMountBlocked
					? TEXT("LegacyCompatible Mount의 stable identity/reference가 구조적으로 유효하지 않습니다.")
					: FString::Printf(TEXT("LegacyCompatible Mount %d개를 custom/multi-Mount 의미 그대로 보존합니다."), Recipe.MountIntents.Num()),
				bLegacyMountBlocked
					? TEXT("Advanced에서 None/duplicate MountProfileId 또는 invalid LocationSlotRef만 교정하세요. Standard 1:1로 자동 변환하지 않습니다.")
					: TEXT("MountType/Size/custom identity를 Standard 규칙으로 강제 변경하지 않습니다."),
				TEXT("MountIntents"),
				bLegacyMountBlocked);
		}
	}
	else
	{
		// UseHardpoints Standard lane은 Recipe Hardpoint 하나당 exact Mount 하나를 요구합니다.
		TSet<FName> KnownHardpointIds;
		bool bHardpointIdentityInvalid = false;
		for (const FCFHardpointIntent& HardpointIntent : Recipe.HardpointIntents)
		{
			if (HardpointIntent.LocationSlotId.IsNone() || KnownHardpointIds.Contains(HardpointIntent.LocationSlotId))
			{
				bHardpointIdentityInvalid = true;
				continue;
			}
			KnownHardpointIds.Add(HardpointIntent.LocationSlotId);
		}

		TSet<FName> SeenMountIds;
		TMap<FName, int32> MountCountByHardpoint;
		int32 StructuralBlockerCount = bHardpointIdentityInvalid ? 1 : 0;
		int32 RuleBlockerCount = 0;
		int32 PresetBlockerCount = 0;
		for (const FCFMountIntent& MountIntent : Recipe.MountIntents)
		{
			if (MountIntent.MountProfileId.IsNone()
				|| SeenMountIds.Contains(MountIntent.MountProfileId)
				|| MountIntent.LocationSlotRef.IsNone()
				|| !KnownHardpointIds.Contains(MountIntent.LocationSlotRef))
			{
				++StructuralBlockerCount;
				continue;
			}
			SeenMountIds.Add(MountIntent.MountProfileId);
			++MountCountByHardpoint.FindOrAdd(MountIntent.LocationSlotRef);

			if (MountIntent.MountType == ECFVehicleMountType::None
				|| (MountIntent.MountType != ECFVehicleMountType::Utility && MountIntent.SizeLimit == ECFVehicleWeaponSize::None))
			{
				++RuleBlockerCount;
			}

			const FSoftObjectPath PresetPath = MountIntent.DefaultEquipmentPresetData.ToSoftObjectPath();
			if (PresetPath.IsValid())
			{
				UObject* PresetObject = PresetPath.ResolveObject();
				if (!PresetObject)
				{
					PresetObject = PresetPath.TryLoad();
				}
				const UCFEquipmentPresetData* PresetData = Cast<UCFEquipmentPresetData>(PresetObject);
				if (!PresetData || !PresetData->CanUseOnMount(MountIntent.MountType, MountIntent.SizeLimit))
				{
					++PresetBlockerCount;
				}
			}
		}

		int32 MissingMountCount = 0;
		int32 MultiMountCount = 0;
		for (const FName HardpointId : KnownHardpointIds)
		{
			const int32 MountCount = MountCountByHardpoint.FindRef(HardpointId);
			if (MountCount == 0)
			{
				++MissingMountCount;
			}
			else if (MountCount > 1)
			{
				++MultiMountCount;
			}
		}

		if (StructuralBlockerCount > 0 || MultiMountCount > 0 || RuleBlockerCount > 0 || PresetBlockerCount > 0)
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::MountProfiles,
				ECFBuilderGuidanceState::Blocked,
				FString::Printf(
					TEXT("Standard 1:1 Mount blocker: structure=%d, multi=%d, rule=%d, preset=%d."),
					StructuralBlockerCount, MultiMountCount, RuleBlockerCount, PresetBlockerCount),
				TEXT("각 Hardpoint에 stable Mount 1개만 두고 MountType을 지정하세요. Fixed/Gimbal/Turret/Launcher는 SizeLimit이 필요하고 Utility는 None을 허용합니다. Preset이 있으면 current CanUseOnMount 호환성을 통과해야 합니다."),
				TEXT("MountIntents"),
				true);
		}
		else if (KnownHardpointIds.IsEmpty() || MissingMountCount > 0 || Recipe.MountIntents.Num() != KnownHardpointIds.Num())
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::MountProfiles,
				ECFBuilderGuidanceState::NeedsReview,
				FString::Printf(
					TEXT("Standard Mount 작성이 덜 끝났습니다. Hardpoint=%d / Mount=%d / missing=%d."),
					KnownHardpointIds.Num(), Recipe.MountIntents.Num(), MissingMountCount),
				KnownHardpointIds.IsEmpty()
					? TEXT("Step 3에서 장착 위치를 하나 이상 추가하세요.")
					: TEXT("각 Hardpoint row에서 MountType/SizeLimit을 선택하고 '장착 규칙 반영'으로 1:1 Mount를 작성하세요."),
				TEXT("MountIntents"),
				true);
		}
		else
		{
			CFVehicleBuilderGuidePrivate::AddGuidanceItem(
				OutResult,
				ECFBuilderGameplayArea::MountProfiles,
				ECFBuilderGuidanceState::Complete,
				FString::Printf(TEXT("Standard 1:1 Mount %d개가 Hardpoint %d개와 exact 대응하고 Type/Size/Preset 호환성을 통과했습니다."), Recipe.MountIntents.Num(), KnownHardpointIds.Num()),
				TEXT("MountProfileId는 stable identity로 유지합니다. EquipmentPresetData는 optional이며 없는 상태도 유효합니다."),
				TEXT("MountIntents"),
				false);
		}
	}

	if (Recipe.DriveStateMode == ECFVehicleDriveStateMode::ProjectDefault)
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::DriveState,
			ECFBuilderGuidanceState::Complete,
			TEXT("DriveState는 Project Default를 사용합니다."),
			TEXT("일반 차량의 기본 선택입니다. 차량별 상태 판정이 실제로 필요할 때만 VehicleSpecific으로 전환하세요."),
			TEXT("DriveStateMode"),
			false);
	}
	else if (!Profiles.DriveStateSource.IsPresent())
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::DriveState,
			ECFBuilderGuidanceState::Blocked,
			TEXT("DriveStateMode가 VehicleSpecific이지만 DriveState Profile이 없습니다."),
			TEXT("기존 DriveState Profile을 명시적으로 연결하거나 Project Default로 되돌리세요. Builder는 DriveState Profile을 자동 생성하지 않습니다."),
			TEXT("ProfileBindings.DriveStateProfile"),
			true);
	}
	else
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::DriveState,
			ECFBuilderGuidanceState::Complete,
			TEXT("VehicleSpecific DriveState Profile이 명시적으로 연결되어 있습니다."),
			TEXT("추가 작업이 필요하지 않습니다."),
			TEXT("DriveStateMode / ProfileBindings.DriveStateProfile"),
			false);
	}

	// VehicleBase WheelVisual payload validation 실패 이유입니다.
	FString WheelVisualFailure;
	if (!Profiles.BaseSource.IsPresent())
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::WheelVisual,
			ECFBuilderGuidanceState::Blocked,
			TEXT("WheelVisual baseline을 제공할 VehicleBase Profile source가 없습니다."),
			TEXT("Builder-private VehicleBase Profile binding을 복구하세요. Wheel 자동검출은 수행하지 않습니다."),
			TEXT("WheelVisualIntent"),
			false);
	}
	else if (!CFVehicleBuilderGuidePrivate::IsWheelVisualProfileComplete(Profiles.BaseData, Recipe.WheelVisualIntent.Mode, WheelVisualFailure))
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::WheelVisual,
			ECFBuilderGuidanceState::Blocked,
			WheelVisualFailure,
			TEXT("ExpectedWheelCount/Steering count와 현재 WheelVisual mode에 실제 필요한 항목만 교정하세요. Socket/Manual mode에서는 Legacy AutoScale clamp를 요구하지 않습니다."),
			TEXT("WheelVisualIntent / Profile.VehicleBase"),
			true);
	}
	else
	{
		// 현재 semantic WheelVisual mode를 사람이 읽는 문자열입니다.
		FString WheelVisualSummary;
		switch (Recipe.WheelVisualIntent.Mode)
		{
		case ECFWheelVisualIntentMode::ManualMeshScale:
			WheelVisualSummary = TEXT("휠 메시 스케일은 수동 정책을 사용합니다.");
			break;
		case ECFWheelVisualIntentMode::AutoScaleToPhysicsRadius:
			WheelVisualSummary = TEXT("휠 메시를 current physics radius에 맞추는 기존 auto-scale 정책을 사용합니다.");
			break;
		case ECFWheelVisualIntentMode::SocketScaleFromChassis:
			WheelVisualSummary = TEXT("차체 Wheel Socket Scale이 타이어 크기 Authority입니다. USER가 휠하우스를 보고 X/Z 직경 배율과 Y 폭 배율을 직접 결정하며 Builder는 크기를 자동 추정·보정하지 않습니다.");
			break;
		case ECFWheelVisualIntentMode::UseProfilePolicy:
		default:
			WheelVisualSummary = Profiles.BaseData.bAutoScaleWheelMeshToRadius
				? TEXT("VehicleBase Profile의 auto-scale 정책을 사용합니다.")
				: TEXT("VehicleBase Profile의 수동 스케일 정책을 사용합니다.");
			break;
		}

		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::WheelVisual,
			ECFBuilderGuidanceState::Complete,
			WheelVisualSummary,
			TEXT("Wheel 중심 자동검출은 하지 않습니다. Mesh/Socket 준비와 실제 휠 위치는 VB-P0-03에서 확정한 USER/Asset authority를 그대로 사용합니다."),
			TEXT("WheelVisualIntent"),
			false);
	}

	// BaseVehicleMassKg의 current semantic source 값입니다.
	const float EffectiveBaseMassKg = Recipe.MassIntent.BaseMassMode == ECFAuthoringInputMode::ExplicitValue
		? Recipe.MassIntent.ExplicitBaseMassKg
		: Profiles.BaseData.BaseVehicleMassKg;
	// MaximumGrossMassKg의 current semantic source 값입니다.
	const float EffectiveGrossMassKg = Recipe.MassIntent.GrossMassMode == ECFAuthoringInputMode::ExplicitValue
		? Recipe.MassIntent.ExplicitGrossMassKg
		: Profiles.BaseData.MaximumGrossMassKg;
	// 두 Fitting mass 값이 모두 legacy/unconfigured 0인지 여부입니다.
	const bool bBothFittingMassUnset = FMath::IsNearlyZero(EffectiveBaseMassKg) && FMath::IsNearlyZero(EffectiveGrossMassKg);
	// 두 Fitting mass 값이 current contract에 맞는지 여부입니다.
	const bool bFittingMassValid = FMath::IsFinite(EffectiveBaseMassKg)
		&& FMath::IsFinite(EffectiveGrossMassKg)
		&& EffectiveBaseMassKg > 0.0f
		&& EffectiveGrossMassKg >= EffectiveBaseMassKg;

	if ((Recipe.MassIntent.BaseMassMode == ECFAuthoringInputMode::UseProfile
			|| Recipe.MassIntent.GrossMassMode == ECFAuthoringInputMode::UseProfile)
		&& !Profiles.BaseSource.IsPresent())
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::FittingMass,
			ECFBuilderGuidanceState::Blocked,
			TEXT("피팅 질량이 VehicleBase Profile을 사용하도록 되어 있지만 Profile source가 없습니다."),
			TEXT("VehicleBase Profile binding을 복구하거나 Base/Gross mass를 명시 값으로 전환하세요."),
			TEXT("MassIntent"),
			false);
	}
	else if (bBothFittingMassUnset)
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::FittingMass,
			ECFBuilderGuidanceState::Optional,
			TEXT("BaseVehicleMassKg와 MaximumGrossMassKg가 모두 0인 미설정 상태입니다."),
			TEXT("현재 contract에서 피팅을 사용하지 않으면 유효합니다. 실제 VehicleFittingData 출격 피팅을 사용할 때만 두 실제 질량 값을 함께 결정하세요. Builder는 Gross mass를 추정하지 않습니다."),
			TEXT("MassIntent"),
			false);
	}
	else if (!bFittingMassValid)
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::FittingMass,
			ECFBuilderGuidanceState::Blocked,
			TEXT("피팅 질량 계약이 불완전합니다. 두 값은 함께 유효해야 하고 Gross mass는 Base mass 이상이어야 합니다."),
			TEXT("피팅을 쓰지 않으면 둘 다 0으로 두고, 사용할 차량이면 실제 BaseVehicleMassKg와 MaximumGrossMassKg를 모두 명시하세요."),
			TEXT("BaseVehicleMassKg / MaximumGrossMassKg"),
			true);
	}
	else
	{
		CFVehicleBuilderGuidePrivate::AddGuidanceItem(
			OutResult,
			ECFBuilderGameplayArea::FittingMass,
			ECFBuilderGuidanceState::Complete,
			FString::Printf(TEXT("피팅 질량 기준이 Base %.2f kg / Gross %.2f kg로 완전합니다."), EffectiveBaseMassKg, EffectiveGrossMassKg),
			TEXT("VehicleFittingData는 필요할 때 이 기준을 read-only snapshot으로 사용합니다."),
			TEXT("BaseVehicleMassKg / MaximumGrossMassKg"),
			false);
	}

	// current Target과 resolved candidate 사이 VB-P0-06 관련 pending diff 수입니다.
	OutResult.PendingGameplayDiffCount = 0;
	for (const FCFVehicleFieldDiff& Diff : ResolveResult.FieldDiff)
	{
		if (CFVehicleBuilderGuidePrivate::IsGameplayDiff(Diff))
		{
			++OutResult.PendingGameplayDiffCount;
		}
	}

	if (Request.Mode == ECFBuilderCompanionMode::CompleteExisting)
	{
		OutResult.ExistingCompletionSummary = OutResult.PendingGameplayDiffCount == 0
			? TEXT("Existing Vehicle의 Gameplay 영역은 current Target과 current authoring candidate가 동일합니다. P0-06에서 Target mutation은 없습니다.")
			: FString::Printf(
				TEXT("Existing Vehicle의 Gameplay 영역에 pending diff %d건이 있습니다. P0-06은 read-only로만 보고하며 실제 Definition Apply는 VB-P0-07 Final Review가 소유합니다."),
				OutResult.PendingGameplayDiffCount);
	}
	else
	{
		OutResult.ExistingCompletionSummary = TEXT("New Vehicle mode입니다. Gameplay Setup guidance는 current authoring candidate를 읽기만 하며 Target Apply/Save를 수행하지 않습니다.");
	}

	OutResult.bCanCompleteGameplayStep = OutResult.BlockedCount == 0 && OutResult.NeedsReviewCount == 0;
	OutResult.Operation.Message = OutResult.bCanCompleteGameplayStep
		? TEXT("Builder Gameplay Setup completeness를 read-only로 평가했습니다. 현재 Step 6을 기술적으로 완료할 수 있습니다.")
		: FString::Printf(
			TEXT("Builder Gameplay Setup completeness를 read-only로 평가했습니다. USER review %d / blocked %d 영역을 해결해야 합니다."),
			OutResult.NeedsReviewCount,
			OutResult.BlockedCount);
	return true;
}
