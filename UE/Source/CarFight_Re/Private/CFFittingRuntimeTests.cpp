// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-08-04
// Description: CF-FQ-033~034 출격 피팅 Runtime Apply·Initial Mass·Defense Commit 자동화 테스트
// Scope: Pawn 없는 단위 계약, 실제 BP/DataAsset Game World 수명과 실제 M_VehicleDefensePIE의 1-Pawn·2-Pawn PIE 방어 초기화를 검증합니다.
// Changelog:
// - v1.5.0: 저장되지 않는 임시 PlayerStart로 플레이어 차량과 대상 SUV가 함께 존재하는 DefenseMapTwoPawnPIE를 추가하고 Fitting·Defense 컴포넌트 단일 인스턴스와 getter 동일성을 검증.
// - v1.4.0: 실제 M_VehicleDefensePIE를 열고 PIE 복제 SUV의 BeginPlay·Initial Mass·Snapshot Commit·방어 초기값을 검증하는 DefenseMapPIE 회귀를 추가.
// - v1.3.0: VehicleMesh 실제 집계 질량의 양의 오버헤드 허용과 Target 아래 허용 오차 초과 실패 계약을 단위·실제 BP 회귀에 고정.
// - v1.2.0: 실제 BP_CFVehiclePawn과 SUV 피팅 DataAsset을 Game World에서 초기화해 ActiveDefenseData까지 검증하는 DefensePIEPipeline 회귀를 추가.
// - v1.1.1: 13.6kg 부동소수점 표현 오차를 허용하도록 허용 오차 단언의 비교 정밀도를 0.001kg로 명시.
// - v1.1.0: FIT_P0_05 InitialMass 상태 계약, 1%·1kg 허용 오차, Verify Only, 다른 질량 거부와 Invalid Legacy fallback 검증을 추가.
// - v1.0.0: RuntimeApply와 AtomicBoundary 테스트를 최초 추가.
// Migration:
// - 기존 단위 테스트는 Transient UObject와 Fake Adapter 계약을 유지합니다.
// - DefensePIEPipeline은 임시 테스트 World에서 실제 Blueprint와 DataAsset을 읽기만 하며 에셋이나 Map을 저장·수정하지 않습니다.
// - DefenseMapPIE는 M_VehicleDefensePIE를 읽기 전용으로 로드하고 실제 PIE 복제 Actor의 저장된 인스턴스 값과 BeginPlay 결과를 검증한 뒤 PIE를 종료합니다.
// - DefenseMapTwoPawnPIE는 에디터 World에 RF_Transient PlayerStart를 임시 추가해 GameMode 플레이어 차량과 맵 배치 SUV가 함께 존재하는 수명을 검증하고 종료 뒤 제거합니다.
// - 실제 차량은 컴포넌트 등록을 해제한 뒤 맵 인스턴스 값을 주입하고 재등록해 PreRegister → Physics State → Runtime Commit 순서를 재현합니다.
// - VehicleMesh 실제 집계 질량은 Snapshot Target보다 큰 PhysicsAsset 오버헤드를 허용하지만 Target보다 허용 오차 이상 부족하면 실패해야 합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFFittingTypes.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFWeaponData.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"

namespace
{
	/** 테스트용 완성 장비 프리셋 묶음입니다. */
	struct FCFFittingRuntimeTestEquipment
	{
		// [v1.0.0] Snapshot에서 비교할 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] 장비 프리셋이 참조할 TurretMountData입니다.
		UCFTurretMountData* TurretMountData = nullptr;

		// [v1.0.0] 장비 프리셋이 참조할 WeaponData입니다.
		UCFWeaponData* WeaponData = nullptr;
	};

	/** Pawn 없이 적용 호출과 실패 Rollback을 기록하는 Fake Adapter입니다. */
	class FCFFittingRuntimeTestAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.0.0] 다음 Snapshot Defense 적용만 실패시킬지 여부입니다.
		bool bFailNextSnapshotDefenseApply = false;

		// [v1.0.0] Weapon 입력 적용 호출 횟수입니다.
		int32 WeaponApplyCallCount = 0;

		// [v1.0.0] Defense 입력 적용 호출 횟수입니다.
		int32 DefenseApplyCallCount = 0;

		// [v1.0.0] 마지막 Weapon 입력입니다.
		FCFFittingWeaponRuntimeInput LastWeaponInput;

		// [v1.0.0] 마지막 Defense 입력입니다.
		FCFFittingDefenseRuntimeInput LastDefenseInput;

		// [v1.0.0] Weapon 입력을 기록하고 적용 성공을 반환합니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			++WeaponApplyCallCount;
			LastWeaponInput = WeaponInput;
			bOutWeaponRuntimeReady = WeaponInput.UsesLegacyVehicleConfiguration() || WeaponInput.bHasResolvedMount;
			return true;
		}

		// [v1.0.0] Defense 입력을 기록하고 요청된 실패를 재현합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			++DefenseApplyCallCount;
			LastDefenseInput = DefenseInput;
			if (bFailNextSnapshotDefenseApply && !DefenseInput.UsesLegacyVehicleConfiguration())
			{
				bFailNextSnapshotDefenseApply = false;
				bOutDefenseRuntimeReady = false;
				return false;
			}

			bOutDefenseRuntimeReady = DefenseInput.DefenseData != nullptr;
			return true;
		}
	};

	// [v1.0.0] 지정 ID와 질량을 가진 완성 장비 프리셋을 생성합니다.
	FCFFittingRuntimeTestEquipment CreateRuntimeTestEquipment(UObject* Outer, const FName EquipmentId, const float TurretMountMassKg, const float WeaponMassKg)
	{
		FCFFittingRuntimeTestEquipment TestEquipment;
		TestEquipment.TurretMountData = NewObject<UCFTurretMountData>(Outer);
		TestEquipment.TurretMountData->TurretMountId = FName(*FString::Printf(TEXT("%s_Mount"), *EquipmentId.ToString()));
		TestEquipment.TurretMountData->TurretMountWeightKg = TurretMountMassKg;
		TestEquipment.WeaponData = NewObject<UCFWeaponData>(Outer);
		TestEquipment.WeaponData->WeaponId = FName(*FString::Printf(TEXT("%s_Weapon"), *EquipmentId.ToString()));
		TestEquipment.WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
		TestEquipment.WeaponData->CompatibleMountTypes.Reset();
		TestEquipment.WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		TestEquipment.WeaponData->WeaponMassKg = WeaponMassKg;
		TestEquipment.EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		TestEquipment.EquipmentPresetData->EquipmentId = EquipmentId;
		TestEquipment.EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		TestEquipment.EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
		TestEquipment.EquipmentPresetData->DefaultTurretMountData = TestEquipment.TurretMountData;
		TestEquipment.EquipmentPresetData->DefaultWeaponData = TestEquipment.WeaponData;
		return TestEquipment;
	}

	// [v1.0.0] 한 하드포인트와 터렛 MountProfile을 가진 VehicleData를 생성합니다.
	UCFVehicleData* CreateRuntimeTestVehicleData(UObject* Outer, UCFEquipmentPresetData* DefaultEquipmentPresetData, UCFVehicleDefenseData* DefaultDefenseData)
	{
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 2500.0f;
		VehicleData->DefaultDefenseData = DefaultDefenseData;

		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Test");
		VehicleData->HardpointSlots.Add(HardpointSlot);

		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret");
		MountProfile.LocationSlotRef = TEXT("Top_01");
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Medium;
		MountProfile.DefaultEquipmentPresetData = DefaultEquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
		return VehicleData;
	}

	// [v1.0.0] 지정 장비와 방어를 Override하는 유효 FittingData를 생성합니다.
	UCFVehicleFittingData* CreateRuntimeTestFittingData(UObject* Outer, UCFVehicleData* VehicleData, const FName FittingId, UCFEquipmentPresetData* EquipmentPresetData, UCFVehicleDefenseData* DefenseData)
	{
		UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(Outer);
		FittingData->FittingId = FittingId;
		FittingData->VehicleData = VehicleData;
		FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
		FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::Override;
		FittingData->DefenseSelection.DefenseData = DefenseData;

		FCFVehicleMountSelection MountSelection;
		MountSelection.MountProfileId = TEXT("RoofTurret");
		MountSelection.EquipmentPresetData = EquipmentPresetData;
		MountSelection.bEnabled = true;
		FittingData->MountSelections.Add(MountSelection);
		return FittingData;
	}

	/** 임시 PlayerStart의 에디터 World 수명과 원래 Package dirty 상태를 공유합니다. */
	struct FCFTransientPlayerStartState
	{
		// [v1.5.0] 에디터 World에 임시로 생성한 PlayerStart입니다.
		TWeakObjectPtr<APlayerStart> TransientPlayerStart;

		// [v1.5.0] 임시 Actor 생성 전 에디터 맵 Package의 dirty 상태입니다.
		bool bEditorMapWasDirty = false;
	};

	/** PIE 시작 전에 저장되지 않는 임시 PlayerStart를 에디터 World에 추가합니다. */
	class FCFAddTransientPlayerStartCommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.5.0] 결과를 기록할 테스트와 cleanup이 공유할 상태를 보존합니다.
		FCFAddTransientPlayerStartCommand(FAutomationTestBase* InTest, const TSharedRef<FCFTransientPlayerStartState>& InSharedState)
			: Test(InTest)
			, SharedState(InSharedState)
		{
		}

		// [v1.5.0] 현재 에디터 맵에 RF_Transient PlayerStart를 한 개 생성합니다.
		virtual bool Update() override
		{
			// [v1.5.0] 다음 PIE가 복제할 현재 에디터 World입니다.
			UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
			if (!EditorWorld)
			{
				Test->AddError(TEXT("CF-FQ-033 DefenseMapTwoPawnPIE: 에디터 World를 찾지 못했습니다."));
				return true;
			}

			// [v1.5.0] 임시 Actor 생성 전에 보존할 현재 맵 Package입니다.
			UPackage* EditorMapPackage = EditorWorld->GetOutermost();
			SharedState->bEditorMapWasDirty = EditorMapPackage && EditorMapPackage->IsDirty();

			// [v1.5.0] 테스트 전부터 존재하는 PlayerStart 수입니다.
			int32 ExistingPlayerStartCount = 0;
			for (TActorIterator<APlayerStart> PlayerStartIterator(EditorWorld); PlayerStartIterator; ++PlayerStartIterator)
			{
				if (IsValid(*PlayerStartIterator))
				{
					++ExistingPlayerStartCount;
				}
			}

			if (ExistingPlayerStartCount > 0)
			{
				Test->AddInfo(FString::Printf(TEXT("CF-FQ-033 DefenseMapTwoPawnPIE | ExistingPlayerStartCount=%d | TransientSpawn=Skipped"), ExistingPlayerStartCount));
				return true;
			}

			// [v1.5.0] 기존 맵과 충돌하지 않는 임시 PlayerStart 생성 설정입니다.
			FActorSpawnParameters PlayerStartSpawnParameters;
			PlayerStartSpawnParameters.Name = MakeUniqueObjectName(EditorWorld, APlayerStart::StaticClass(), TEXT("CF_AutomationPlayerStart"));
			PlayerStartSpawnParameters.ObjectFlags |= RF_Transient;
			PlayerStartSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// [v1.5.0] 테스트 SUV와 충분히 떨어진 원점 부근의 임시 플레이어 시작 Transform입니다.
			const FTransform PlayerStartTransform(FRotator::ZeroRotator, FVector(0.0, 0.0, 150.0));
			// [v1.5.0] PIE 복제와 GameMode 기본 Pawn 생성을 유도할 임시 PlayerStart입니다.
			APlayerStart* TransientPlayerStart = EditorWorld->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), PlayerStartTransform, PlayerStartSpawnParameters);
			if (!TransientPlayerStart)
			{
				Test->AddError(TEXT("CF-FQ-033 DefenseMapTwoPawnPIE: 임시 PlayerStart 생성에 실패했습니다."));
				return true;
			}

			TransientPlayerStart->Tags.AddUnique(TEXT("CF_AutomationTransientPlayerStart"));
			SharedState->TransientPlayerStart = TransientPlayerStart;
			Test->AddInfo(FString::Printf(
				TEXT("CF-FQ-033 DefenseMapTwoPawnPIE | TransientPlayerStart=%s | Flags=%u | MapWasDirty=%s"),
				*TransientPlayerStart->GetPathName(),
				static_cast<uint32>(TransientPlayerStart->GetFlags()),
				SharedState->bEditorMapWasDirty ? TEXT("Yes") : TEXT("No")));
			return true;
		}

	private:
		// [v1.5.0] 오류와 관찰 정보를 기록할 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
		// [v1.5.0] PIE 종료 cleanup과 공유할 임시 PlayerStart 상태입니다.
		TSharedRef<FCFTransientPlayerStartState> SharedState;
	};

	/** PIE 종료 뒤 임시 PlayerStart를 제거하고 원래 맵 dirty 상태를 복원합니다. */
	class FCFCleanupTransientPlayerStartCommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.5.0] 결과를 기록할 테스트와 제거할 임시 Actor 상태를 보존합니다.
		FCFCleanupTransientPlayerStartCommand(FAutomationTestBase* InTest, const TSharedRef<FCFTransientPlayerStartState>& InSharedState)
			: Test(InTest)
			, SharedState(InSharedState)
			, StartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// [v1.5.0] PIE World 정리가 끝날 때까지 기다린 뒤 에디터 World 임시 Actor를 제거합니다.
		virtual bool Update() override
		{
			// [v1.5.0] 아직 살아 있는 PIE World가 있는지 여부입니다.
			bool bHasActivePIEWorld = false;
			if (GEngine)
			{
				for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
				{
					if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
					{
						bHasActivePIEWorld = true;
						break;
					}
				}
			}

			// [v1.5.0] PIE 종료 대기 제한 시간을 계산할 경과 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			if (bHasActivePIEWorld && ElapsedSeconds < 15.0)
			{
				return false;
			}

			// [v1.5.0] 임시 Actor가 생성됐던 원본 에디터 World입니다.
			UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
			if (!EditorWorld)
			{
				Test->AddError(TEXT("CF-FQ-033 DefenseMapTwoPawnPIE: cleanup 에디터 World를 찾지 못했습니다."));
				return true;
			}

			// [v1.5.0] 공유 상태에 남아 있는 임시 PlayerStart입니다.
			APlayerStart* TransientPlayerStart = SharedState->TransientPlayerStart.Get();
			if (IsValid(TransientPlayerStart))
			{
				TransientPlayerStart->Destroy();
				SharedState->TransientPlayerStart.Reset();
			}

			// [v1.5.0] 테스트 전 깨끗한 맵이었다면 임시 Actor 추가·제거로 생긴 dirty 플래그를 되돌릴 Package입니다.
			UPackage* EditorMapPackage = EditorWorld->GetOutermost();
			if (EditorMapPackage && !SharedState->bEditorMapWasDirty)
			{
				EditorMapPackage->SetDirtyFlag(false);
			}

			Test->AddInfo(FString::Printf(
				TEXT("CF-FQ-033 DefenseMapTwoPawnPIE | Cleanup=Completed | ActivePIEWorld=%s | MapDirty=%s"),
				bHasActivePIEWorld ? TEXT("Yes") : TEXT("No"),
				EditorMapPackage && EditorMapPackage->IsDirty() ? TEXT("Yes") : TEXT("No")));
			return true;
		}

	private:
		// [v1.5.0] cleanup 오류와 관찰 정보를 기록할 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
		// [v1.5.0] 제거할 임시 PlayerStart 상태입니다.
		TSharedRef<FCFTransientPlayerStartState> SharedState;
		// [v1.5.0] PIE 종료 cleanup 제한 시간을 계산할 시작 시각입니다.
		double StartTimeSeconds = 0.0;
	};

	/** 실제 PIE World에서 맵 배치 방어 SUV의 BeginPlay 초기화 결과를 검증합니다. */
	class FCFVerifyDefenseMapPIECommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.5.0] 검증 결과를 기록할 Automation Test와 플레이어 차량 요구 여부를 보존합니다.
		FCFVerifyDefenseMapPIECommand(FAutomationTestBase* InTest, const bool bInRequirePlayerVehicle)
			: Test(InTest)
			, bRequirePlayerVehicle(bInRequirePlayerVehicle)
			, StartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// [v1.4.0] PIE World 준비를 기다린 뒤 실제 맵 복제 SUV의 피팅·방어 초기값을 검증합니다.
		virtual bool Update() override
		{
			// [v1.4.0] 현재 Engine World Context에서 찾은 실제 PIE World입니다.
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

			// [v1.4.0] PIE World 준비를 기다린 누적 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			// [v1.4.0] 맵 또는 GameMode 시작 실패를 무한 대기하지 않을 제한 시간입니다.
			constexpr double PIEWorldReadyTimeoutSeconds = 30.0;
			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if (ElapsedSeconds >= PIEWorldReadyTimeoutSeconds)
				{
					Test->AddError(TEXT("CF-FQ-033 DefenseMapPIE: 30초 안에 PIE World Actor 초기화가 완료되지 않았습니다."));
					return true;
				}
				return false;
			}

			// [v1.4.0] 저장된 맵 인스턴스에서 테스트 SUV를 식별할 정확한 FittingData 경로입니다.
			const FString ExpectedFittingDataPath = TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV");
			// [v1.4.0] PIE World에서 발견한 전체 CFVehiclePawn 수입니다.
			int32 VehiclePawnCount = 0;
			// [v1.5.0] PlayerController에 빙의된 차량 Pawn 수입니다.
			int32 PlayerControlledVehiclePawnCount = 0;
			// [v1.5.0] PIE World의 모든 차량 경로·제어·데이터 상태를 기록할 요약입니다.
			FString VehiclePawnInventorySummary;
			// [v1.4.0] 정확한 FittingData를 가진 실제 PIE 복제 방어 SUV입니다.
			ACFVehiclePawn* DefenseVehiclePawn = nullptr;
			for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(PIEWorld); VehiclePawnIterator; ++VehiclePawnIterator)
			{
				// [v1.4.0] 현재 순회 중인 PIE 차량 Pawn입니다.
				ACFVehiclePawn* CandidateVehiclePawn = *VehiclePawnIterator;
				if (!IsValid(CandidateVehiclePawn))
				{
					continue;
				}

				++VehiclePawnCount;
				if (CandidateVehiclePawn->IsPlayerControlled())
				{
					++PlayerControlledVehiclePawnCount;
				}

				if (!VehiclePawnInventorySummary.IsEmpty())
				{
					VehiclePawnInventorySummary += TEXT("; ");
				}

				VehiclePawnInventorySummary += FString::Printf(
					TEXT("%s{PlayerControlled=%s,Controller=%s,VehicleData=%s,FittingData=%s}"),
					*CandidateVehiclePawn->GetPathName(),
					CandidateVehiclePawn->IsPlayerControlled() ? TEXT("Yes") : TEXT("No"),
					*GetPathNameSafe(CandidateVehiclePawn->GetController()),
					*GetPathNameSafe(CandidateVehiclePawn->VehicleData.Get()),
					*GetPathNameSafe(CandidateVehiclePawn->VehicleFittingData.Get()));

				// [v1.4.0] 현재 차량 Pawn에 직렬화된 FittingData 경로입니다.
				const FString CandidateFittingDataPath = GetPathNameSafe(CandidateVehiclePawn->VehicleFittingData.Get());
				if (CandidateFittingDataPath == ExpectedFittingDataPath)
				{
					DefenseVehiclePawn = CandidateVehiclePawn;
				}
			}

			if (bRequirePlayerVehicle && (VehiclePawnCount < 2 || PlayerControlledVehiclePawnCount < 1))
			{
				if (ElapsedSeconds < PIEWorldReadyTimeoutSeconds)
				{
					return false;
				}

				Test->AddError(FString::Printf(
					TEXT("CF-FQ-033 DefenseMapTwoPawnPIE: 30초 안에 플레이어 차량과 대상 SUV 2-Pawn 구성이 준비되지 않았습니다. PawnCount=%d, PlayerControlled=%d, Pawns=%s"),
					VehiclePawnCount,
					PlayerControlledVehiclePawnCount,
					*VehiclePawnInventorySummary));
				return true;
			}

			if (!DefenseVehiclePawn)
			{
				Test->AddError(FString::Printf(
					TEXT("CF-FQ-033 DefenseMapPIE: PIE 차량 %d대 중 DA_Fit_DefenseTestSUV를 가진 맵 복제 Actor를 찾지 못했습니다. Pawns=%s"),
					VehiclePawnCount,
					*VehiclePawnInventorySummary));
				return true;
			}

			// [v1.4.0] Initial Mass와 Snapshot Commit 상태를 제공하는 실제 PIE 피팅 컴포넌트입니다.
			UCFVehicleFittingComp* VehicleFittingComp = DefenseVehiclePawn->GetVehicleFittingComp();
			// [v1.4.0] ActiveDefenseData와 Shield·Armor 초기값을 제공하는 실제 PIE 방어 컴포넌트입니다.
			UCFVehicleDefenseComp* VehicleDefenseComp = DefenseVehiclePawn->GetVehicleDefenseComp();
			// [v1.4.0] 차량 Integrity 초기값을 제공하는 실제 PIE 내구도 컴포넌트입니다.
			UCFVehicleHealthComp* VehicleHealthComp = DefenseVehiclePawn->GetVehicleHealthComp();
			// [v1.4.0] 테스트 SUV의 기본 방어 DataAsset을 제공하는 실제 PIE VehicleData입니다.
			UCFVehicleData* VehicleData = DefenseVehiclePawn->VehicleData.Get();
			// [v1.5.0] 대상 SUV가 실제로 보유한 모든 VehicleFittingComp 인스턴스입니다.
			TArray<UCFVehicleFittingComp*> VehicleFittingComponents;
			DefenseVehiclePawn->GetComponents<UCFVehicleFittingComp>(VehicleFittingComponents);
			// [v1.5.0] 대상 SUV가 실제로 보유한 모든 VehicleDefenseComp 인스턴스입니다.
			TArray<UCFVehicleDefenseComp*> VehicleDefenseComponents;
			DefenseVehiclePawn->GetComponents<UCFVehicleDefenseComp>(VehicleDefenseComponents);

			Test->AddInfo(FString::Printf(
				TEXT("CF-FQ-033 DefenseMapPIE | World=%s | RequirePlayer=%s | PawnCount=%d | PlayerControlled=%d | Pawns=%s | Actor=%s | BegunPlay=%s | AutoInit=%s | VehicleData=%s | FittingData=%s | FittingGetter=%s@%p | FittingCount=%d | DefenseGetter=%s@%p | DefenseCount=%d | InitialMass=%s | Apply=%s | DefenseReady=%s | ActiveDefense=%s | Shield=%.3f/%.3f | Integrity=%.3f | MassSummary=%s | RuntimeSummary=%s"),
				*PIEWorld->GetPathName(),
				bRequirePlayerVehicle ? TEXT("Yes") : TEXT("No"),
				VehiclePawnCount,
				PlayerControlledVehiclePawnCount,
				*VehiclePawnInventorySummary,
				*DefenseVehiclePawn->GetPathName(),
				DefenseVehiclePawn->HasActorBegunPlay() ? TEXT("Yes") : TEXT("No"),
				DefenseVehiclePawn->bAutoInitializeOnBeginPlay ? TEXT("Yes") : TEXT("No"),
				*GetPathNameSafe(VehicleData),
				*GetPathNameSafe(DefenseVehiclePawn->VehicleFittingData.Get()),
				*GetPathNameSafe(VehicleFittingComp),
				static_cast<const void*>(VehicleFittingComp),
				VehicleFittingComponents.Num(),
				*GetPathNameSafe(VehicleDefenseComp),
				static_cast<const void*>(VehicleDefenseComp),
				VehicleDefenseComponents.Num(),
				VehicleFittingComp ? *UEnum::GetValueAsString(VehicleFittingComp->GetInitialMassState()) : TEXT("Missing"),
				VehicleFittingComp ? *UEnum::GetValueAsString(VehicleFittingComp->GetRuntimeApplyState()) : TEXT("Missing"),
				VehicleDefenseComp && VehicleDefenseComp->IsDefenseInitialized() ? TEXT("Yes") : TEXT("No"),
				VehicleDefenseComp ? *GetPathNameSafe(VehicleDefenseComp->GetActiveDefenseData()) : TEXT("Missing"),
				VehicleDefenseComp ? VehicleDefenseComp->GetCurrentShield() : 0.0f,
				VehicleDefenseComp ? VehicleDefenseComp->GetMaximumShield() : 0.0f,
				VehicleHealthComp ? VehicleHealthComp->GetCurrentIntegrity() : 0.0f,
				VehicleFittingComp ? *VehicleFittingComp->GetLastInitialMassSummary() : TEXT("MissingFittingComp"),
				VehicleFittingComp ? *VehicleFittingComp->GetLastFittingRuntimeSummary() : TEXT("MissingFittingComp")));

			if (bRequirePlayerVehicle)
			{
				Test->TestTrue(TEXT("2-Pawn PIE 차량 수 2대 이상"), VehiclePawnCount >= 2);
				Test->TestTrue(TEXT("2-Pawn PIE 플레이어 제어 차량 존재"), PlayerControlledVehiclePawnCount >= 1);
			}
			Test->TestEqual(TEXT("맵 복제 SUV VehicleFittingComp 인스턴스 1개"), VehicleFittingComponents.Num(), 1);
			Test->TestTrue(TEXT("맵 복제 SUV Fitting getter는 실제 단일 인스턴스"), VehicleFittingComponents.Num() == 1 && VehicleFittingComponents[0] == VehicleFittingComp);
			Test->TestEqual(TEXT("맵 복제 SUV VehicleDefenseComp 인스턴스 1개"), VehicleDefenseComponents.Num(), 1);
			Test->TestTrue(TEXT("맵 복제 SUV Defense getter는 실제 단일 인스턴스"), VehicleDefenseComponents.Num() == 1 && VehicleDefenseComponents[0] == VehicleDefenseComp);
			Test->TestTrue(TEXT("맵 복제 SUV BeginPlay 완료"), DefenseVehiclePawn->HasActorBegunPlay());
			Test->TestTrue(TEXT("맵 복제 SUV 자동 초기화 활성"), DefenseVehiclePawn->bAutoInitializeOnBeginPlay);
			Test->TestTrue(TEXT("맵 복제 SUV VehicleData 연결"), VehicleData != nullptr);
			Test->TestTrue(TEXT("맵 복제 SUV FittingData 연결"), GetPathNameSafe(DefenseVehiclePawn->VehicleFittingData.Get()) == ExpectedFittingDataPath);
			Test->TestTrue(TEXT("맵 복제 SUV Initial Mass 검증 통과"), VehicleFittingComp && VehicleFittingComp->HasVerifiedInitialMass());
			Test->TestTrue(TEXT("맵 복제 SUV Snapshot Commit 완료"), VehicleFittingComp && VehicleFittingComp->HasAppliedFittingSnapshot());
			Test->TestTrue(TEXT("맵 복제 SUV Weapon Commit 준비"), VehicleFittingComp && VehicleFittingComp->WasLastWeaponRuntimeReady());
			Test->TestTrue(TEXT("맵 복제 SUV Defense Commit 준비"), VehicleFittingComp && VehicleFittingComp->WasLastDefenseRuntimeReady());
			Test->TestTrue(TEXT("맵 복제 SUV Defense 초기화 완료"), VehicleDefenseComp && VehicleDefenseComp->IsDefenseInitialized());
			Test->TestTrue(
				TEXT("맵 복제 SUV ActiveDefenseData는 VehicleData 기본 방어"),
				VehicleData && VehicleDefenseComp && VehicleDefenseComp->GetActiveDefenseData() == VehicleData->DefaultDefenseData.Get());

			// [v1.4.0] 방어층 초기값의 공통 예상 수치입니다.
			constexpr float ExpectedDefenseStartValue = 100.0f;
			// [v1.4.0] 부동소수점 시작 수치 비교 허용 오차입니다.
			constexpr float DefenseValueTolerance = 0.001f;
			Test->TestTrue(TEXT("맵 복제 SUV Maximum Shield 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetMaximumShield(), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Current Shield 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentShield(), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Front Armor 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentArmor(ECFArmorDirection::Front), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Left Armor 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentArmor(ECFArmorDirection::Left), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Right Armor 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentArmor(ECFArmorDirection::Right), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Rear Armor 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentArmor(ECFArmorDirection::Rear), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Top Armor 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentArmor(ECFArmorDirection::Top), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Bottom Armor 100"), VehicleDefenseComp && FMath::IsNearlyEqual(VehicleDefenseComp->GetCurrentArmor(ECFArmorDirection::Bottom), ExpectedDefenseStartValue, DefenseValueTolerance));
			Test->TestTrue(TEXT("맵 복제 SUV Integrity 100"), VehicleHealthComp && FMath::IsNearlyEqual(VehicleHealthComp->GetCurrentIntegrity(), ExpectedDefenseStartValue, DefenseValueTolerance));
			return true;
		}

	private:
				// [v1.4.0] Latent 검증 결과를 기록할 현재 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
		// [v1.5.0] GameMode 플레이어 차량까지 반드시 존재해야 하는 검증인지 여부입니다.
		bool bRequirePlayerVehicle = false;
		// [v1.4.0] PIE World 준비 제한 시간을 계산할 시작 시각입니다.
		double StartTimeSeconds = 0.0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingRuntimeApplyTest, "CarFight.Fitting.FIT_P0_04.RuntimeApply", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingAtomicBoundaryTest, "CarFight.Fitting.FIT_P0_04.AtomicBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingInitialMassTest, "CarFight.Fitting.FIT_P0_05.InitialMass", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingDefensePIEPipelineTest, "CarFight.Fitting.FIT_P0_05.DefensePIEPipeline", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingDefenseMapPIETest, "CarFight.Fitting.FIT_P0_05.DefenseMapPIE", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingDefenseMapTwoPawnPIETest, "CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Legacy, Snapshot Commit, 재초기화와 Reset 수명을 검증합니다.
bool FCFFittingRuntimeApplyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	FCFFittingRuntimeTestEquipment LegacyEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("LegacyEquipment"), 80.0f, 120.0f);
	FCFFittingRuntimeTestEquipment OverrideEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("OverrideEquipment"), 90.0f, 140.0f);
	UCFVehicleDefenseData* LegacyDefenseData = NewObject<UCFVehicleDefenseData>();
	LegacyDefenseData->DefenseId = TEXT("LegacyDefense");
	LegacyDefenseData->DefenseMassKg = 100.0f;
	UCFVehicleDefenseData* OverrideDefenseData = NewObject<UCFVehicleDefenseData>();
	OverrideDefenseData->DefenseId = TEXT("OverrideDefense");
	OverrideDefenseData->DefenseMassKg = 130.0f;
	UCFVehicleData* VehicleData = CreateRuntimeTestVehicleData(GetTransientPackage(), LegacyEquipment.EquipmentPresetData, LegacyDefenseData);
	UCFVehicleFittingComp* FittingComp = NewObject<UCFVehicleFittingComp>();
	FCFFittingRuntimeTestAdapter RuntimeAdapter;

	TestTrue(TEXT("Legacy Prepare 성공"), FittingComp->PrepareSortieFitting(nullptr, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("Legacy Prepared 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::PreparedLegacy);
	TestEqual(TEXT("Prepare Weapon 무호출"), RuntimeAdapter.WeaponApplyCallCount, 0);
	TestTrue(TEXT("Legacy Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("Legacy Committed 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::CommittedLegacy);
	TestFalse(TEXT("Legacy Applied Snapshot 없음"), FittingComp->HasAppliedFittingSnapshot());

	UCFVehicleFittingData* FittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("RuntimeOverride"), OverrideEquipment.EquipmentPresetData, OverrideDefenseData);
	TestTrue(TEXT("Snapshot Prepare 성공"), FittingComp->PrepareSortieFitting(FittingData, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("Snapshot Prepare Weapon 무변경"), RuntimeAdapter.WeaponApplyCallCount, 1);
	TestTrue(TEXT("Snapshot Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestTrue(TEXT("Applied Snapshot 존재"), FittingComp->HasAppliedFittingSnapshot());
	TestEqual(TEXT("Applied Fitting ID"), FittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("RuntimeOverride")));
	TestEqual(TEXT("장비 Override 전달"), RuntimeAdapter.LastWeaponInput.EquipmentPresetData.Get(), OverrideEquipment.EquipmentPresetData);
	TestEqual(TEXT("방어 Override 전달"), RuntimeAdapter.LastDefenseInput.DefenseData.Get(), OverrideDefenseData);

	TestTrue(TEXT("재Prepare 성공"), FittingComp->PrepareSortieFitting(FittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("재Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("재초기화 Mount 수 1"), FittingComp->GetAppliedFittingSnapshot().ResolvedMounts.Num(), 1);

	FittingComp->ResetFittingRuntimeState();
	TestEqual(TEXT("Reset 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::Uninitialized);
	TestFalse(TEXT("Reset Applied 없음"), FittingComp->HasAppliedRuntimeInput());
	TestFalse(TEXT("Reset Snapshot 없음"), FittingComp->HasAppliedFittingSnapshot());
	return true;
}

// [v1.0.0] 무효 무변경과 부분 실패 Rollback을 검증합니다.
bool FCFFittingAtomicBoundaryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	FCFFittingRuntimeTestEquipment FirstEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("FirstEquipment"), 70.0f, 110.0f);
	FCFFittingRuntimeTestEquipment SecondEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("SecondEquipment"), 75.0f, 115.0f);
	UCFVehicleDefenseData* FirstDefenseData = NewObject<UCFVehicleDefenseData>();
	FirstDefenseData->DefenseId = TEXT("FirstDefense");
	FirstDefenseData->DefenseMassKg = 90.0f;
	UCFVehicleDefenseData* SecondDefenseData = NewObject<UCFVehicleDefenseData>();
	SecondDefenseData->DefenseId = TEXT("SecondDefense");
	SecondDefenseData->DefenseMassKg = 95.0f;
	UCFVehicleData* VehicleData = CreateRuntimeTestVehicleData(GetTransientPackage(), FirstEquipment.EquipmentPresetData, FirstDefenseData);
	UCFVehicleFittingData* FirstFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("FirstApplied"), FirstEquipment.EquipmentPresetData, FirstDefenseData);
	UCFVehicleFittingData* SecondFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("SecondCandidate"), SecondEquipment.EquipmentPresetData, SecondDefenseData);
	UCFVehicleFittingComp* FittingComp = NewObject<UCFVehicleFittingComp>();
	FCFFittingRuntimeTestAdapter RuntimeAdapter;

	TestTrue(TEXT("첫 Prepare 성공"), FittingComp->PrepareSortieFitting(FirstFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("첫 Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	FirstFittingData->MountSelections.Reset();
	const int32 WeaponCallsBeforeInvalid = RuntimeAdapter.WeaponApplyCallCount;
	const int32 DefenseCallsBeforeInvalid = RuntimeAdapter.DefenseApplyCallCount;
	TestFalse(TEXT("무효 Snapshot Prepare 거부"), FittingComp->PrepareSortieFitting(FirstFittingData, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("무효 Prepare Weapon 무호출"), RuntimeAdapter.WeaponApplyCallCount, WeaponCallsBeforeInvalid);
	TestEqual(TEXT("무효 Prepare Defense 무호출"), RuntimeAdapter.DefenseApplyCallCount, DefenseCallsBeforeInvalid);
	TestEqual(TEXT("기존 Applied 유지"), FittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("FirstApplied")));

	TestTrue(TEXT("두 번째 Prepare 성공"), FittingComp->PrepareSortieFitting(SecondFittingData, VehicleData, TEXT("RoofTurret")));
	RuntimeAdapter.bFailNextSnapshotDefenseApply = true;
	TestFalse(TEXT("Defense 실패 Commit 거부"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("실패 뒤 Applied 유지"), FittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("FirstApplied")));
	TestEqual(TEXT("Rollback Weapon 첫 장비"), RuntimeAdapter.LastWeaponInput.EquipmentPresetData.Get(), FirstEquipment.EquipmentPresetData);
	TestEqual(TEXT("Rollback Defense 첫 방어"), RuntimeAdapter.LastDefenseInput.DefenseData.Get(), FirstDefenseData);
	TestFalse(TEXT("실패 뒤 Prepared 없음"), FittingComp->HasPreparedRuntimeInput());

	TestTrue(TEXT("명시 Rollback Prepare"), FittingComp->PrepareSortieFitting(SecondFittingData, VehicleData, TEXT("RoofTurret")));
	const int32 WeaponCallsBeforeRollback = RuntimeAdapter.WeaponApplyCallCount;
	FittingComp->RollbackPreparedSortieFitting();
	TestEqual(TEXT("명시 Rollback 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::RolledBack);
	TestEqual(TEXT("명시 Rollback Adapter 무호출"), RuntimeAdapter.WeaponApplyCallCount, WeaponCallsBeforeRollback);
	return true;
}

// [v1.1.0] Legacy 유지, Snapshot 초기 질량, 허용 오차, Verify Only와 다른 질량 재적용 거부를 검증합니다.
bool FCFFittingInitialMassTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] Legacy와 Snapshot 경로가 공유할 기본 장비 프리셋입니다.
	FCFFittingRuntimeTestEquipment LegacyEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("MassLegacyEquipment"), 80.0f, 120.0f);
	// [v1.1.0] 초기 출격 Snapshot Target을 만들 Override 장비 프리셋입니다.
	FCFFittingRuntimeTestEquipment OverrideEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("MassOverrideEquipment"), 90.0f, 140.0f);
	// [v1.1.0] 다른 Runtime Mass 요청을 재현할 중량 장비 프리셋입니다.
	FCFFittingRuntimeTestEquipment HeavyEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("MassHeavyEquipment"), 180.0f, 260.0f);
	// [v1.1.0] VehicleData Legacy 방어 패키지입니다.
	UCFVehicleDefenseData* LegacyDefenseData = NewObject<UCFVehicleDefenseData>();
	LegacyDefenseData->DefenseId = TEXT("MassLegacyDefense");
	LegacyDefenseData->DefenseMassKg = 100.0f;
	// [v1.1.0] 유효 Snapshot의 방어 패키지입니다.
	UCFVehicleDefenseData* OverrideDefenseData = NewObject<UCFVehicleDefenseData>();
	OverrideDefenseData->DefenseId = TEXT("MassOverrideDefense");
	OverrideDefenseData->DefenseMassKg = 130.0f;
	// [v1.1.0] 다른 Target Mass를 만들 중량 방어 패키지입니다.
	UCFVehicleDefenseData* HeavyDefenseData = NewObject<UCFVehicleDefenseData>();
	HeavyDefenseData->DefenseId = TEXT("MassHeavyDefense");
	HeavyDefenseData->DefenseMassKg = 220.0f;
	// [v1.1.0] 모든 Initial Mass 시나리오가 공유할 차량 플랫폼입니다.
	UCFVehicleData* VehicleData = CreateRuntimeTestVehicleData(GetTransientPackage(), LegacyEquipment.EquipmentPresetData, LegacyDefenseData);
	// [v1.1.0] 1000 + 90 + 140 + 130으로 1360kg Target을 만들 피팅입니다.
	UCFVehicleFittingData* OverrideFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("MassOverride"), OverrideEquipment.EquipmentPresetData, OverrideDefenseData);
	// [v1.1.0] 이미 Physics State가 생성된 뒤 거부할 더 무거운 피팅입니다.
	UCFVehicleFittingData* HeavyFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("MassHeavy"), HeavyEquipment.EquipmentPresetData, HeavyDefenseData);

	// [v1.1.0] FittingData 미지정 시 기존 Chaos 질량과 Legacy Weapon·Defense 입력을 유지할 컴포넌트입니다.
	UCFVehicleFittingComp* LegacyFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("Legacy Initial Prepare 성공"), LegacyFittingComp->PrepareInitialSortieFitting(nullptr, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("Legacy Initial Mass 사용"), LegacyFittingComp->UsesLegacyInitialMass());
	TestFalse(TEXT("Legacy Mass 기록 불필요"), LegacyFittingComp->ShouldApplyPreparedInitialMass());
	TestTrue(TEXT("Legacy Mass 검증은 기존 질량 유지로 통과"), LegacyFittingComp->VerifyInitialMassAfterPhysics(0.0f, 0.0f, false, false, false));
	TestEqual(TEXT("Legacy Mass 상태"), LegacyFittingComp->GetInitialMassState(), ECFInitialMassState::LegacyPreserved);

	// [v1.1.0] 유효 Snapshot의 초기 질량 상태와 Runtime Commit을 검증할 컴포넌트입니다.
	UCFVehicleFittingComp* SnapshotFittingComp = NewObject<UCFVehicleFittingComp>();
	FCFFittingRuntimeTestAdapter RuntimeAdapter;
	TestTrue(TEXT("Snapshot Initial Prepare 성공"), SnapshotFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestFalse(TEXT("Snapshot은 Legacy Mass 아님"), SnapshotFittingComp->UsesLegacyInitialMass());
	TestTrue(TEXT("첫 Snapshot Mass 기록 필요"), SnapshotFittingComp->ShouldApplyPreparedInitialMass());
	// [v1.1.0] Snapshot 공식으로 계산된 예상 초기 출격 질량입니다.
	const float ExpectedTargetMassKg = 1360.0f;
	TestTrue(TEXT("Snapshot Target 1360kg"), FMath::IsNearlyEqual(SnapshotFittingComp->GetPreparedInitialMassKg(), ExpectedTargetMassKg));
	// [v1.1.0] 1360kg Target의 1% 허용 오차입니다.
	const float ExpectedToleranceKg = 13.6f;
	TestTrue(TEXT("Initial Mass 허용 오차 13.6kg"), FMath::IsNearlyEqual(UCFVehicleFittingComp::CalculateInitialMassToleranceKg(ExpectedTargetMassKg), ExpectedToleranceKg, 0.001f));
	TestTrue(TEXT("물리 생성 전 Movement Mass 기록"), SnapshotFittingComp->RecordInitialMassBeforePhysics(1500.0f, ExpectedTargetMassKg));
	TestTrue(TEXT("Configured Initial Mass 존재"), SnapshotFittingComp->HasConfiguredInitialMass());
	TestTrue(TEXT("실제 질량 허용 오차 안 검증 성공"), SnapshotFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg + 5.0f, true, true, true));
	TestTrue(TEXT("Initial Mass 검증 완료"), SnapshotFittingComp->HasVerifiedInitialMass());
	TestTrue(TEXT("검증된 Cached Snapshot Commit 성공"), SnapshotFittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("검증 뒤 Applied Fitting ID"), SnapshotFittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassOverride")));

	// [v1.3.0] PhysicsAsset 보조 Body 집계로 실제 질량이 Target보다 커지는 경우를 독립적으로 검증할 컴포넌트입니다.
	UCFVehicleFittingComp* AuxiliaryBodyMassFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("집계 질량 시나리오 Snapshot Prepare"), AuxiliaryBodyMassFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("집계 질량 시나리오 PrePhysics 기록"), AuxiliaryBodyMassFittingComp->RecordInitialMassBeforePhysics(1500.0f, ExpectedTargetMassKg));
	TestTrue(TEXT("실제 질량 양의 오버헤드는 Target Coverage로 검증 성공"), AuxiliaryBodyMassFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg + 250.0f, true, true, true));
	TestEqual(TEXT("집계 질량 Coverage 상태"), AuxiliaryBodyMassFittingComp->GetInitialMassState(), ECFInitialMassState::Verified);

	TestTrue(TEXT("같은 Snapshot 재초기화 Prepare 성공"), SnapshotFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestFalse(TEXT("같은 질량 재초기화는 Mass 재기록 없음"), SnapshotFittingComp->ShouldApplyPreparedInitialMass());
	TestTrue(TEXT("같은 질량 재초기화 Verify Only 성공"), SnapshotFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg, true, true, true));
	SnapshotFittingComp->RollbackPreparedSortieFitting();

	TestFalse(TEXT("다른 Target Mass 재적용 거부"), SnapshotFittingComp->PrepareInitialSortieFitting(HeavyFittingData, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("다른 질량 거부 상태"), SnapshotFittingComp->GetInitialMassState(), ECFInitialMassState::ReapplyRejected);
	TestFalse(TEXT("다른 질량 거부 뒤 Prepared 입력 없음"), SnapshotFittingComp->HasPreparedRuntimeInput());
	TestEqual(TEXT("다른 질량 거부 뒤 기존 Applied 유지"), SnapshotFittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassOverride")));

	// [v1.3.0] VehicleMesh 실제 집계 질량이 Target보다 부족한 전파 실패를 독립적으로 검증할 컴포넌트입니다.
	UCFVehicleFittingComp* FailedMassFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("실패 시나리오 Snapshot Prepare"), FailedMassFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("실패 시나리오 PrePhysics 기록"), FailedMassFittingComp->RecordInitialMassBeforePhysics(1500.0f, ExpectedTargetMassKg));
	TestFalse(TEXT("실제 질량이 Target 아래 허용 오차 밖이면 검증 실패"), FailedMassFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg - 50.0f, true, true, true));
	TestEqual(TEXT("실제 질량 부족 실패 상태"), FailedMassFittingComp->GetInitialMassState(), ECFInitialMassState::VerificationFailed);

	// [v1.1.0] 초기 Invalid Snapshot이 Legacy 입력으로 fallback하는지 확인할 독립 피팅입니다.
	UCFVehicleFittingData* InvalidFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("MassInvalid"), OverrideEquipment.EquipmentPresetData, OverrideDefenseData);
	InvalidFittingData->MountSelections.Reset();
	UCFVehicleFittingComp* InvalidFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("초기 Invalid Snapshot Legacy fallback 성공"), InvalidFittingComp->PrepareInitialSortieFitting(InvalidFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("Invalid Snapshot Legacy 질량 유지"), InvalidFittingComp->UsesLegacyInitialMass());
	TestEqual(TEXT("Invalid Snapshot Legacy Prepared 상태"), InvalidFittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::PreparedLegacy);

	SnapshotFittingComp->ResetFittingRuntimeState();
	TestFalse(TEXT("Reset Initial Mass 구성 없음"), SnapshotFittingComp->HasConfiguredInitialMass());
	TestFalse(TEXT("Reset Initial Mass 검증 없음"), SnapshotFittingComp->HasVerifiedInitialMass());
	TestEqual(TEXT("Reset Initial Mass 상태"), SnapshotFittingComp->GetInitialMassState(), ECFInitialMassState::Uninitialized);
	return true;
}

// [v1.2.0] 실제 BP와 DataAsset을 Game World 수명으로 초기화해 Defense Commit까지 검증합니다.
bool FCFFittingDefensePIEPipelineTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] 에셋을 저장하지 않고 Actor 등록과 Physics State 생성을 수행할 임시 테스트 World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("임시 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.2.0] PIE와 같은 PreRegister Initial Mass 경로를 활성화하고 함수 종료 시 원래 World 유형을 복원합니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// [v1.2.0] PIE 테스트 맵과 같은 실제 차량 Blueprint 생성 클래스입니다.
	UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));

	// [v1.2.0] DefaultDefenseData가 테스트 방어 DataAsset을 가리키는 SUV 차량 DataAsset입니다.
	UCFVehicleData* VehicleDataAsset = LoadObject<UCFVehicleData>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"));

	// [v1.2.0] UseVehicleDefault 방어 선택과 SUV 장비 선택을 가진 실제 피팅 DataAsset입니다.
	UCFVehicleFittingData* VehicleFittingDataAsset = LoadObject<UCFVehicleFittingData>(
		nullptr,
		TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV"));

	if (!TestNotNull(TEXT("BP_CFVehiclePawn 클래스 로드"), VehiclePawnClass)
		|| !TestNotNull(TEXT("DA_VehicleDefense_TestSUV 로드"), VehicleDataAsset)
		|| !TestNotNull(TEXT("DA_Fit_DefenseTestSUV 로드"), VehicleFittingDataAsset))
	{
		return false;
	}

	TestNotNull(TEXT("SUV VehicleData DefaultDefenseData 연결"), VehicleDataAsset->DefaultDefenseData.Get());
	TestTrue(TEXT("피팅 VehicleData는 SUV VehicleData"), VehicleFittingDataAsset->VehicleData == VehicleDataAsset);

	// [v1.2.0] Actor 수명과 독립적으로 실제 로드 에셋의 호환성·질량 계약을 먼저 확인할 Snapshot입니다.
	const FCFVehicleFittingSnapshot LoadedFittingSnapshot = VehicleFittingDataAsset->BuildFittingSnapshot();
	AddInfo(FString::Printf(
		TEXT("CF-FQ-033 LoadedSnapshot | State=%s | Issues=%d | Mounts=%d | TotalMass=%.3f | Defense=%s"),
		*UEnum::GetValueAsString(LoadedFittingSnapshot.ValidationState),
		LoadedFittingSnapshot.ValidationIssues.Num(),
		LoadedFittingSnapshot.ResolvedMounts.Num(),
		LoadedFittingSnapshot.TotalVehicleMassKg,
		*GetPathNameSafe(LoadedFittingSnapshot.ResolvedDefenseData.Get())));
	for (const FCFFittingValidationIssue& ValidationIssue : LoadedFittingSnapshot.ValidationIssues)
	{
		AddInfo(FString::Printf(
			TEXT("CF-FQ-033 SnapshotIssue | Severity=%s | Code=%s | Mount=%s | Message=%s"),
			*UEnum::GetValueAsString(ValidationIssue.Severity),
			*UEnum::GetValueAsString(ValidationIssue.IssueCode),
			*ValidationIssue.MountProfileId.ToString(),
			*ValidationIssue.Message.ToString()));
	}
	TestTrue(TEXT("로드된 SUV Fitting Snapshot 유효"), LoadedFittingSnapshot.IsValid());

	// [v1.2.0] 실제 Blueprint 컴포넌트 계층과 Chaos VehicleMesh를 생성할 테스트 차량입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		VehiclePawnClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!TestNotNull(TEXT("SUV 피팅 테스트 차량 Spawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.2.0] 최초 Blueprint 기본값으로 생성된 Physics State를 제거해 맵 인스턴스 직렬화 이전 단계를 재현합니다.
	VehiclePawn->UnregisterAllComponents();

	// [v1.2.0] 맵 Actor에 저장된 것과 같은 SUV VehicleData 인스턴스 값입니다.
	VehiclePawn->VehicleData = VehicleDataAsset;

	// [v1.2.0] 맵 Actor에 저장된 것과 같은 SUV VehicleFittingData 인스턴스 값입니다.
	VehiclePawn->VehicleFittingData = VehicleFittingDataAsset;
	VehiclePawn->bAutoInitializeOnBeginPlay = true;
	VehiclePawn->bAutoRegisterInputMappingContext = false;
	VehiclePawn->bShowAimReticle = false;
	VehiclePawn->bShowTargetSelectHud = false;

	// [v1.2.0] 최초 Blueprint 기본값으로 준비된 Legacy 상태를 지우고 SUV Snapshot 수명을 새로 시작합니다.
	UCFVehicleFittingComp* PreRegisterFittingComp = VehiclePawn->GetVehicleFittingComp();
	if (PreRegisterFittingComp)
	{
		PreRegisterFittingComp->ResetFittingRuntimeState();
	}

	// [v1.2.0] PreRegister에서 Snapshot 1570kg을 기록하고 새 Physics State를 생성합니다.
	VehiclePawn->RegisterAllComponents();

	AddInfo(FString::Printf(
		TEXT("CF-FQ-033 RegisteredPawnData | VehicleData=%s | FittingData=%s | InitialMass=%s | MassSummary=%s"),
		*GetPathNameSafe(VehiclePawn->VehicleData.Get()),
		*GetPathNameSafe(VehiclePawn->VehicleFittingData.Get()),
		PreRegisterFittingComp ? *UEnum::GetValueAsString(PreRegisterFittingComp->GetInitialMassState()) : TEXT("Missing"),
		PreRegisterFittingComp ? *PreRegisterFittingComp->GetLastInitialMassSummary() : TEXT("MissingFittingComp")));
	TestTrue(TEXT("등록 차량 VehicleData는 SUV DataAsset"), VehiclePawn->VehicleData == VehicleDataAsset);
	TestTrue(TEXT("등록 차량 VehicleFittingData는 SUV FittingData"), VehiclePawn->VehicleFittingData == VehicleFittingDataAsset);
	TestTrue(TEXT("PreRegister Snapshot 질량 구성 완료"), PreRegisterFittingComp && PreRegisterFittingComp->HasConfiguredInitialMass());

	// [v1.2.0] BeginPlay가 호출하는 동일 Runtime 초기화 진입점의 최종 성공 여부입니다.
	const bool bVehicleRuntimeInitialized = VehiclePawn->InitializeVehicleRuntime();

	// [v1.2.0] Snapshot·Initial Mass·Weapon·Defense 단계 상태를 제공하는 실제 피팅 컴포넌트입니다.
	UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();

	// [v1.2.0] 최종 ActiveDefenseData와 방어 초기화 상태를 제공하는 실제 방어 컴포넌트입니다.
	UCFVehicleDefenseComp* VehicleDefenseComp = VehiclePawn->GetVehicleDefenseComp();

	// [v1.2.0] Snapshot Target이 기록된 Chaos Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* VehicleMovementComp = Cast<UChaosWheeledVehicleMovementComponent>(
		VehiclePawn->GetVehicleMovementComponent());

	// [v1.2.0] Physics State가 보고하는 실제 차량 질량을 제공하는 SkeletalMesh 루트입니다.
	USkeletalMeshComponent* VehicleMeshComp = VehiclePawn->GetMesh();

	AddInfo(FString::Printf(
		TEXT("CF-FQ-033 Pipeline | Runtime=%s | InitialMass=%s | Apply=%s | Configured=%.3f | Actual=%.3f | PhysicsState=%s | Simulate=%s | PhysicsAsset=%s | WeaponReady=%s | DefenseReady=%s | ActiveDefense=%s | MassSummary=%s | RuntimeSummary=%s"),
		bVehicleRuntimeInitialized ? TEXT("Ready") : TEXT("Failed"),
		VehicleFittingComp ? *UEnum::GetValueAsString(VehicleFittingComp->GetInitialMassState()) : TEXT("Missing"),
		VehicleFittingComp ? *UEnum::GetValueAsString(VehicleFittingComp->GetRuntimeApplyState()) : TEXT("Missing"),
		VehicleMovementComp ? VehicleMovementComp->Mass : 0.0f,
		VehicleMeshComp ? VehicleMeshComp->GetMass() : 0.0f,
		VehicleMeshComp && VehicleMeshComp->IsPhysicsStateCreated() ? TEXT("Yes") : TEXT("No"),
		VehicleMeshComp && VehicleMeshComp->IsSimulatingPhysics() ? TEXT("Yes") : TEXT("No"),
		VehicleMeshComp && VehicleMeshComp->GetPhysicsAsset() ? TEXT("Yes") : TEXT("No"),
		VehicleFittingComp && VehicleFittingComp->WasLastWeaponRuntimeReady() ? TEXT("Yes") : TEXT("No"),
		VehicleFittingComp && VehicleFittingComp->WasLastDefenseRuntimeReady() ? TEXT("Yes") : TEXT("No"),
		VehicleDefenseComp ? *GetPathNameSafe(VehicleDefenseComp->GetActiveDefenseData()) : TEXT("MissingDefenseComp"),
		VehicleFittingComp ? *VehicleFittingComp->GetLastInitialMassSummary() : TEXT("MissingFittingComp"),
		VehicleFittingComp ? *VehicleFittingComp->GetLastFittingRuntimeSummary() : TEXT("MissingFittingComp")));

	TestNotNull(TEXT("VehicleFittingComp 존재"), VehicleFittingComp);
	TestNotNull(TEXT("VehicleDefenseComp 존재"), VehicleDefenseComp);
	TestTrue(TEXT("Fitting Snapshot Initial Mass 검증 통과"), VehicleFittingComp && VehicleFittingComp->HasVerifiedInitialMass());
	TestTrue(TEXT("Fitting Snapshot Commit 완료"), VehicleFittingComp && VehicleFittingComp->HasAppliedFittingSnapshot());
	TestTrue(TEXT("Weapon Commit 준비 완료"), VehicleFittingComp && VehicleFittingComp->WasLastWeaponRuntimeReady());
	TestTrue(TEXT("Defense Commit 준비 완료"), VehicleFittingComp && VehicleFittingComp->WasLastDefenseRuntimeReady());
	TestTrue(TEXT("VehicleDefense 런타임 초기화 완료"), VehicleDefenseComp && VehicleDefenseComp->IsDefenseInitialized());
	TestTrue(
		TEXT("ActiveDefenseData는 VehicleData 기본 방어 DataAsset"),
		VehicleDefenseComp && VehicleDefenseComp->GetActiveDefenseData() == VehicleDataAsset->DefaultDefenseData.Get());

	VehiclePawn->Destroy();
	return true;
}

// [v1.4.0] 실제 M_VehicleDefensePIE를 열고 맵 배치 SUV의 PIE 복제 수명을 검증합니다.
bool FCFFittingDefenseMapPIETest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] 사용자 DR-PIE-00과 같은 저장된 테스트 맵 경로입니다.
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFVerifyDefenseMapPIECommand(this, false));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

// [v1.5.0] 임시 PlayerStart로 플레이어 차량과 맵 배치 SUV가 함께 존재하는 실제 2-Pawn PIE 수명을 검증합니다.
bool FCFFittingDefenseMapTwoPawnPIETest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.5.0] 사용자 수동 PIE와 같은 저장된 VehicleDefense 테스트 맵입니다.
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	// [v1.5.0] 임시 PlayerStart 추가와 PIE 종료 cleanup이 공유할 상태입니다.
	const TSharedRef<FCFTransientPlayerStartState> TransientPlayerStartState = MakeShared<FCFTransientPlayerStartState>();
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FCFAddTransientPlayerStartCommand(this, TransientPlayerStartState));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFVerifyDefenseMapPIECommand(this, true));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FCFCleanupTransientPlayerStartCommand(this, TransientPlayerStartState));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
