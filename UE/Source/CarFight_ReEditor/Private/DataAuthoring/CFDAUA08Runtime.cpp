// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFDAUA08Runtime.cpp
// Version: v1.0.0
// Date: 2026-08-24
// Description: DAUTH-P0-12 UA-08 Driving Feel Runtime Comparison용 transient PIE Sports 비교 harness입니다.
// Scope: Persistent Recipe/VehicleData/Map/Blueprint를 수정하지 않고 PIE Pawn에만 transient resolved VehicleData를 적용합니다.
// Changelog:
// - v1.0.0: UA-07 Sports 4축 Intent와 UA-07 Handling/Performance fixture를 shared Resolver로 resolve하고,
//   Driving Feel이 소유하는 Movement 13개 field만 transient VehicleData에 적용한 뒤 PIE VehiclePawn runtime을 재초기화합니다.
// Migration:
// - Production Authoring 경로가 아닙니다. UA-08 USER Acceptance의 원본 자산 무변경 A/B 비교 전용입니다.
// - CarFight.DAUA08.Sports 명령은 PIE에서 정확히 1회 실행하며, 원복은 PIE 종료로만 수행합니다.
// - Persistent Recipe/VehicleData Save, Apply, Adoption, Package Dirty mutation은 수행하지 않습니다.

#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFDAUA08Runtime, Log, All);

namespace CFDAUA08Runtime
{
	// UA-08에서 read-only source로 복제할 Production Recipe의 정확한 object path입니다.
	const TCHAR* ProductionRecipeObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_TestSedan.DA_Recipe_TestSedan");

	// UA-07/08의 Steering/Grip/Suspension monotonic response를 제공하는 test-only Handling Profile path입니다.
	const TCHAR* HandlingProfileObjectPath = TEXT("/Game/Test/CarFightDataAuthoring/DA_UA07_Handling.DA_UA07_Handling");

	// UA-07/08의 Acceleration monotonic response를 제공하는 test-only Performance Profile path입니다.
	const TCHAR* PerformanceProfileObjectPath = TEXT("/Game/Test/CarFightDataAuthoring/DA_UA07_Performance.DA_UA07_Performance");

	// Sports preset의 가속 반응 4축 값입니다.
	constexpr float SportsAccelerationFeel = 0.85f;

	// Sports preset의 조향 민첩성 4축 값입니다.
	constexpr float SportsSteeringAgility = 0.78f;

	// Sports preset의 접지감 4축 값입니다.
	constexpr float SportsGripFeel = 0.82f;

	// Sports preset의 서스펜션 단단함 4축 값입니다.
	constexpr float SportsSuspensionFirmness = 0.78f;

	// Sports Driving Feel이 실제 runtime 비교에 적용해야 하는 exact Movement leaf 13개입니다.
	const TMap<FString, FName>& GetDrivingFeelMovementFields()
	{
		// Stable Field Path를 FCFVehicleMovementConfig의 실제 leaf property 이름으로 연결하는 고정 표입니다.
		static const TMap<FString, FName> DrivingFeelMovementFields =
		{
			{TEXT("VehicleMovementConfig.EngineMaxTorque"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, EngineMaxTorque)},
			{TEXT("VehicleMovementConfig.EngineMaxRPM"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, EngineMaxRPM)},
			{TEXT("VehicleMovementConfig.ThrottleInputScale"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, ThrottleInputScale)},
			{TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, FrontWheelMaxSteerAngle)},
			{TEXT("VehicleMovementConfig.SteeringAngleRatio"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, SteeringAngleRatio)},
			{TEXT("VehicleMovementConfig.FrontWheelFrictionForceMultiplier"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, FrontWheelFrictionForceMultiplier)},
			{TEXT("VehicleMovementConfig.RearWheelFrictionForceMultiplier"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, RearWheelFrictionForceMultiplier)},
			{TEXT("VehicleMovementConfig.FrontWheelCorneringStiffness"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, FrontWheelCorneringStiffness)},
			{TEXT("VehicleMovementConfig.RearWheelCorneringStiffness"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, RearWheelCorneringStiffness)},
			{TEXT("VehicleMovementConfig.FrontWheelSpringRate"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, FrontWheelSpringRate)},
			{TEXT("VehicleMovementConfig.RearWheelSpringRate"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, RearWheelSpringRate)},
			{TEXT("VehicleMovementConfig.FrontWheelSpringPreload"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, FrontWheelSpringPreload)},
			{TEXT("VehicleMovementConfig.RearWheelSpringPreload"), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, RearWheelSpringPreload)}
		};
		return DrivingFeelMovementFields;
	}

	// 현재 Editor process 안에서 활성 PIE world 하나를 반환합니다.
	UWorld* FindActivePIEWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		// 현재 Engine이 관리하는 world context 목록입니다.
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (const FWorldContext& WorldContext : WorldContexts)
		{
			if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
			{
				return WorldContext.World();
			}
		}
		return nullptr;
	}

	// Handling/Performance group의 Registry path를 수집해 transient Recipe에서 실제 Adoption과 같은 Legacy Pin 해제를 수행합니다.
	bool ApplyTransientDrivingFeelOwnership(UCFVehicleRecipeData& TransientRecipe, int32& OutHandlingPinCount, int32& OutPerformancePinCount, FString& OutError)
	{
		OutHandlingPinCount = 0;
		OutPerformancePinCount = 0;
		OutError.Reset();

		// Handling/Performance Registry scalar path를 실제 Adoption group으로 연결하는 조회표입니다.
		TMap<FString, ECFVehicleAdoptGroup> DrivingFeelGroupByPath;
		for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
		{
			if (Descriptor.AdoptionGroup == ECFVehicleAdoptGroup::Handling || Descriptor.AdoptionGroup == ECFVehicleAdoptGroup::Performance)
			{
				DrivingFeelGroupByPath.Add(Descriptor.GetCanonicalPattern(), Descriptor.AdoptionGroup);
			}
		}

		// 뒤에서 앞으로 제거할 현재 Legacy Pin index입니다.
		int32 PinIndex = TransientRecipe.ImportState.LegacyPinnedFields.Num() - 1;
		for (; PinIndex >= 0; --PinIndex)
		{
			// 현재 Legacy Pin의 canonical exact Stable Field Path입니다.
			const FString CanonicalPinPath = TransientRecipe.ImportState.LegacyPinnedFields[PinIndex].FieldPath.ToCanonicalString(true);

			// 현재 Pin이 Handling/Performance 어느 group인지 나타내는 Registry 결과입니다.
			const ECFVehicleAdoptGroup* AdoptionGroup = DrivingFeelGroupByPath.Find(CanonicalPinPath);
			if (!AdoptionGroup)
			{
				continue;
			}

			if (*AdoptionGroup == ECFVehicleAdoptGroup::Handling)
			{
				++OutHandlingPinCount;
			}
			else if (*AdoptionGroup == ECFVehicleAdoptGroup::Performance)
			{
				++OutPerformancePinCount;
			}

			TransientRecipe.ImportState.LegacyPinnedFields.RemoveAt(PinIndex);
		}

		if (OutHandlingPinCount <= 0 || OutPerformancePinCount <= 0)
		{
			OutError = FString::Printf(
				TEXT("UA-08 transient ownership 준비 실패: HandlingPins=%d PerformancePins=%d"),
				OutHandlingPinCount,
				OutPerformancePinCount);
			return false;
		}

		TransientRecipe.ImportState.AdoptedGroups.Add(ECFVehicleAdoptGroup::Handling);
		TransientRecipe.ImportState.AdoptedGroups.Add(ECFVehicleAdoptGroup::Performance);

		// 실제 group Adoption 뒤 ImportService::UpdateManageState와 같은 transient 관리 상태입니다.
		TransientRecipe.ImportState.ManageState = TransientRecipe.ImportState.LegacyPinnedFields.IsEmpty()
			? ECFVehicleManageState::Managed
			: ECFVehicleManageState::PartiallyManaged;
		return true;
	}

	// Shared Resolver 결과에서 Driving Feel 소유 13개 Movement leaf만 transient VehicleData에 checked canonical import합니다.
	bool ApplyResolvedDrivingFeelFields(const FCFVehicleResolveResult& ResolveResult, UCFVehicleData& TransientVehicleData, FString& OutAppliedSummary, FString& OutError)
	{
		OutAppliedSummary.Reset();
		OutError.Reset();

		// UA-08에 적용해야 하는 Stable Path→Movement property 표입니다.
		const TMap<FString, FName>& DrivingFeelMovementFields = GetDrivingFeelMovementFields();

		// 같은 field를 중복 적용하지 않았는지 검증할 canonical path 집합입니다.
		TSet<FString> AppliedFieldPaths;

		// 로그에 남길 path=value row 목록입니다.
		TArray<FString> AppliedValueRows;
		for (const FCFVehicleResolvedField& ResolvedField : ResolveResult.SortedResolvedFields)
		{
			// Resolver field의 canonical exact path입니다.
			const FString CanonicalFieldPath = ResolvedField.FieldPath.ToCanonicalString(true);

			// 이 field가 UA-08 Driving Feel runtime 비교 대상인지 확인한 Movement property 이름입니다.
			const FName* MovementPropertyName = DrivingFeelMovementFields.Find(CanonicalFieldPath);
			if (!MovementPropertyName)
			{
				continue;
			}

			if (AppliedFieldPaths.Contains(CanonicalFieldPath))
			{
				OutError = FString::Printf(TEXT("UA-08 resolved field 중복: %s"), *CanonicalFieldPath);
				return false;
			}

			// FCFVehicleMovementConfig에서 exact leaf property를 찾은 Reflection descriptor입니다.
			FProperty* MovementProperty = FindFProperty<FProperty>(FCFVehicleMovementConfig::StaticStruct(), *MovementPropertyName);
			if (!MovementProperty)
			{
				OutError = FString::Printf(TEXT("UA-08 Movement property를 찾지 못했습니다: %s"), *MovementPropertyName->ToString());
				return false;
			}

			// transient VehicleData Movement struct 안의 실제 leaf storage 주소입니다.
			void* MovementValueAddress = MovementProperty->ContainerPtrToValuePtr<void>(&TransientVehicleData.VehicleMovementConfig);

			// Canonical Resolver value import 실패 원인을 받을 문자열입니다.
			FString ImportError;
			if (!FCFVehicleFieldCodec::ImportValue(*MovementProperty, MovementValueAddress, &TransientVehicleData, ResolvedField.Value, ImportError))
			{
				OutError = FString::Printf(TEXT("UA-08 resolved value import 실패: %s / %s"), *CanonicalFieldPath, *ImportError);
				return false;
			}

			AppliedFieldPaths.Add(CanonicalFieldPath);
			AppliedValueRows.Add(FString::Printf(TEXT("%s=%s"), *CanonicalFieldPath, *ResolvedField.Value.CanonicalValueText));
		}

		if (AppliedFieldPaths.Num() != DrivingFeelMovementFields.Num())
		{
			OutError = FString::Printf(
				TEXT("UA-08 resolved Driving Feel field coverage 불일치: Applied=%d Expected=%d"),
				AppliedFieldPaths.Num(),
				DrivingFeelMovementFields.Num());
			return false;
		}

		AppliedValueRows.Sort();
		OutAppliedSummary = FString::Join(AppliedValueRows, TEXT(" | "));
		return true;
	}

	// PIE Player VehiclePawn에 Sports transient resolve를 정확히 1회 적용합니다.
	void ApplySportsRuntimeComparison()
	{
		// 현재 활성 PIE world입니다. Editor world에는 절대 적용하지 않습니다.
		UWorld* PIEWorld = FindActivePIEWorld();
		if (!PIEWorld)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=NoActivePIEWorld"));
			return;
		}

		// PIE 첫 Local PlayerController입니다.
		APlayerController* PlayerController = PIEWorld->GetFirstPlayerController();
		if (!PlayerController)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=NoPlayerController"));
			return;
		}

		// 실제 사용자 주행 Pawn을 CarFight VehiclePawn으로 제한합니다.
		ACFVehiclePawn* VehiclePawn = Cast<ACFVehiclePawn>(PlayerController->GetPawn());
		if (!VehiclePawn)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=PlayerPawnIsNotCFVehiclePawn"));
			return;
		}

		// Persistent source로만 읽을 Production Recipe입니다.
		UCFVehicleRecipeData* ProductionRecipe = LoadObject<UCFVehicleRecipeData>(nullptr, ProductionRecipeObjectPath);
		if (!ProductionRecipe)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=ProductionRecipeLoadFailed path=%s"), ProductionRecipeObjectPath);
			return;
		}

		// Recipe가 binding한 실제 baseline persistent VehicleData입니다.
		UCFVehicleData* PersistentTargetVehicleData = ProductionRecipe->TargetVehicleData.LoadSynchronous();
		if (!PersistentTargetVehicleData)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=TargetVehicleDataLoadFailed"));
			return;
		}

		if (VehiclePawn->VehicleData != PersistentTargetVehicleData)
		{
			UE_LOG(
				LogCFDAUA08Runtime,
				Error,
				TEXT("CF_DA_UA08_SPORTS_FAIL reason=UnexpectedPawnVehicleData pawn=%s expected=%s"),
				*GetPathNameSafe(VehiclePawn->VehicleData),
				*GetPathNameSafe(PersistentTargetVehicleData));
			return;
		}

		// Sports Acceleration response를 제공할 UA test-only Performance Profile입니다.
		UCFPerformanceProfile* PerformanceProfile = LoadObject<UCFPerformanceProfile>(nullptr, PerformanceProfileObjectPath);

		// Sports Steering/Grip/Suspension response를 제공할 UA test-only Handling Profile입니다.
		UCFHandlingProfile* HandlingProfile = LoadObject<UCFHandlingProfile>(nullptr, HandlingProfileObjectPath);
		if (!PerformanceProfile || !HandlingProfile)
		{
			UE_LOG(
				LogCFDAUA08Runtime,
				Error,
				TEXT("CF_DA_UA08_SPORTS_FAIL reason=ProfileLoadFailed handling=%s performance=%s"),
				*GetPathNameSafe(HandlingProfile),
				*GetPathNameSafe(PerformanceProfile));
			return;
		}

		// Persistent Recipe package의 시작 dirty 상태입니다. Harness가 이 상태를 바꾸면 실패입니다.
		const bool bRecipeDirtyBefore = ProductionRecipe->GetOutermost()->IsDirty();

		// Persistent Target package의 시작 dirty 상태입니다. Harness가 이 상태를 바꾸면 실패입니다.
		const bool bTargetDirtyBefore = PersistentTargetVehicleData->GetOutermost()->IsDirty();

		// 같은 TransientPackage 안에서 반복 호출 시에도 충돌하지 않을 transient Recipe object 이름입니다.
		const FName TransientRecipeName = MakeUniqueObjectName(GetTransientPackage(), UCFVehicleRecipeData::StaticClass(), TEXT("DAUA08_SportsRecipe"));

		// Production Recipe의 전체 semantic state를 value-equivalent하게 복제한 transient Recipe입니다.
		UCFVehicleRecipeData* TransientRecipe = DuplicateObject<UCFVehicleRecipeData>(ProductionRecipe, GetTransientPackage(), TransientRecipeName);
		if (!TransientRecipe)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=TransientRecipeDuplicateFailed"));
			return;
		}
		TransientRecipe->ClearFlags(RF_Public | RF_Standalone);
		TransientRecipe->SetFlags(RF_Transient);

		TransientRecipe->ProfileBindings.HandlingProfile = TSoftObjectPtr<UCFHandlingProfile>(FSoftObjectPath(HandlingProfile));
		TransientRecipe->ProfileBindings.PerformanceProfile = TSoftObjectPtr<UCFPerformanceProfile>(FSoftObjectPath(PerformanceProfile));
		TransientRecipe->DrivingFeelIntent.AccelerationFeel = SportsAccelerationFeel;
		TransientRecipe->DrivingFeelIntent.SteeringAgility = SportsSteeringAgility;
		TransientRecipe->DrivingFeelIntent.GripFeel = SportsGripFeel;
		TransientRecipe->DrivingFeelIntent.SuspensionFirmness = SportsSuspensionFirmness;

		// transient Handling ownership 전환에서 제거한 Legacy Pin 수입니다.
		int32 RemovedHandlingPinCount = 0;

		// transient Performance ownership 전환에서 제거한 Legacy Pin 수입니다.
		int32 RemovedPerformancePinCount = 0;

		// transient ownership 준비 실패 원인입니다.
		FString OwnershipError;
		if (!ApplyTransientDrivingFeelOwnership(*TransientRecipe, RemovedHandlingPinCount, RemovedPerformancePinCount, OwnershipError))
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=%s"), *OwnershipError);
			return;
		}

		// Shared Authoring facade에 전달할 read-only transient Recipe resolve request입니다.
		FCFVehicleAuthoringReadRequest ResolveRequest;
		ResolveRequest.Recipe = TransientRecipe;
		ResolveRequest.TargetVehicleData = PersistentTargetVehicleData;
		ResolveRequest.CallerKind = ECFAuthoringCallerKind::Automation;

		// Shared SnapshotBuilder + Pure Resolver가 반환할 exact resolve evidence입니다.
		FCFVehicleResolveReadResult ResolveReadResult;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ResolveRequest, ResolveReadResult)
			|| ResolveReadResult.ResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
		{
			UE_LOG(
				LogCFDAUA08Runtime,
				Error,
				TEXT("CF_DA_UA08_SPORTS_FAIL reason=ResolveFailed status=%d message=%s"),
				static_cast<int32>(ResolveReadResult.ResolveResult.ResolveStatus),
				*ResolveReadResult.Operation.Message);
			return;
		}

		// 같은 TransientPackage 안에서 반복 호출 시에도 충돌하지 않을 transient VehicleData object 이름입니다.
		const FName TransientVehicleDataName = MakeUniqueObjectName(GetTransientPackage(), UCFVehicleData::StaticClass(), TEXT("DAUA08_SportsVehicleData"));

		// Persistent Target의 비-DrivingFeel 값과 참조를 그대로 유지할 transient runtime VehicleData입니다.
		UCFVehicleData* TransientVehicleData = DuplicateObject<UCFVehicleData>(PersistentTargetVehicleData, GetTransientPackage(), TransientVehicleDataName);
		if (!TransientVehicleData)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=TransientVehicleDataDuplicateFailed"));
			return;
		}
		TransientVehicleData->ClearFlags(RF_Public | RF_Standalone);
		TransientVehicleData->SetFlags(RF_Transient);

		// 실제 transient VehicleData에 적용된 13개 canonical path/value 요약입니다.
		FString AppliedFieldSummary;

		// Resolver value import 실패 원인입니다.
		FString ApplyFieldsError;
		if (!ApplyResolvedDrivingFeelFields(ResolveReadResult.ResolveResult, *TransientVehicleData, AppliedFieldSummary, ApplyFieldsError))
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=%s"), *ApplyFieldsError);
			return;
		}

		VehiclePawn->VehicleData = TransientVehicleData;

		// 기존 public Vehicle runtime 초기화 경로가 transient VehicleData를 Movement/Input/Drive runtime에 실제 적용했는지 나타냅니다.
		const bool bRuntimeInitialized = VehiclePawn->InitializeVehicleRuntime();
		if (!bRuntimeInitialized)
		{
			UE_LOG(LogCFDAUA08Runtime, Error, TEXT("CF_DA_UA08_SPORTS_FAIL reason=InitializeVehicleRuntimeFailed"));
			return;
		}

		// Persistent Recipe package의 종료 dirty 상태입니다.
		const bool bRecipeDirtyAfter = ProductionRecipe->GetOutermost()->IsDirty();

		// Persistent Target package의 종료 dirty 상태입니다.
		const bool bTargetDirtyAfter = PersistentTargetVehicleData->GetOutermost()->IsDirty();
		if (bRecipeDirtyBefore != bRecipeDirtyAfter || bTargetDirtyBefore != bTargetDirtyAfter)
		{
			UE_LOG(
				LogCFDAUA08Runtime,
				Error,
				TEXT("CF_DA_UA08_SPORTS_FAIL reason=PersistentDirtyStateChanged recipe=%d->%d target=%d->%d"),
				bRecipeDirtyBefore ? 1 : 0,
				bRecipeDirtyAfter ? 1 : 0,
				bTargetDirtyBefore ? 1 : 0,
				bTargetDirtyAfter ? 1 : 0);
			return;
		}

		UE_LOG(
			LogCFDAUA08Runtime,
			Display,
			TEXT("CF_DA_UA08_SPORTS_PASS pawn=%s persistent_target=%s transient_target=%s handling_pins=%d performance_pins=%d axes=%.2f/%.2f/%.2f/%.2f persistent_recipe_dirty=%d persistent_target_dirty=%d"),
			*GetPathNameSafe(VehiclePawn),
			*GetPathNameSafe(PersistentTargetVehicleData),
			*GetPathNameSafe(TransientVehicleData),
			RemovedHandlingPinCount,
			RemovedPerformancePinCount,
			SportsAccelerationFeel,
			SportsSteeringAgility,
			SportsGripFeel,
			SportsSuspensionFirmness,
			bRecipeDirtyAfter ? 1 : 0,
			bTargetDirtyAfter ? 1 : 0);

		UE_LOG(LogCFDAUA08Runtime, Display, TEXT("CF_DA_UA08_SPORTS_FIELDS %s"), *AppliedFieldSummary);
	}

	// UA-08 changed-state PIE에서 사용자/AI가 명시적으로 1회 호출할 console command입니다.
	static FAutoConsoleCommand ApplySportsRuntimeComparisonCommand(
		TEXT("CarFight.DAUA08.Sports"),
		TEXT("UA-08 test-only: active PIE VehiclePawn에 Sports 4축 transient resolved VehicleData를 1회 적용합니다. Persistent Asset은 수정/저장하지 않습니다."),
		FConsoleCommandDelegate::CreateStatic(&ApplySportsRuntimeComparison));
}
