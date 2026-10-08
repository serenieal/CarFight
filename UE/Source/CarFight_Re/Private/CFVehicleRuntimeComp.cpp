// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-16
// Description: CF-FQ-048 VPS-P0-03 차량 Runtime orchestration + VehicleData 기본 Sensor wiring
// Changelog:
// - v1.2.0: invalid VehicleData.DefaultSensorData 적용 실패를 무시하지 않고 초기화를 fail-closed해 stale 기본 Sensor source가 남는 재초기화 경로를 차단.
// - v1.1.0: Fitting Scanner 적용 전에 VehicleData.DefaultSensorData를 VehicleSensorComp의 차량 기본 Sensor Source로 전달.
// - v1.0.0: Initialize/Refresh, Initial Mass prepare/verify, VehicleData 적용 순서를 Pawn 공개 표면 변경 없이 내부 coordinator로 추출.
// Migration:
// - ACFVehiclePawn의 lifecycle/Public/BP/Automation 진입점은 그대로 유지되며 이 컴포넌트는 내부 실행만 담당합니다.
// - 기존 VehicleData의 DefaultSensorData=None은 기존 zero-range Fallback을 그대로 유지합니다. Scanner fitting이 있으면 Scanner SensorData가 기본 Sensor보다 우선합니다.
// - v1.2.0부터 명시된 기본 SensorData가 invalid면 이전 Runtime source를 재사용하지 않고 InitializeVehicleRuntime 자체가 실패합니다.

#include "CFVehicleRuntimeComp.h"

#include "CFCombatFxComp.h"
#include "CFLauncherComp.h"
#include "CFTargetSelectComp.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleDriveComp.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleSensorComp.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

// [v1.0.0] 독립 Tick 없이 Pawn lifecycle에서만 호출되는 내부 Runtime coordinator를 초기화합니다.
UCFVehicleRuntimeComp::UCFVehicleRuntimeComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// [v1.0.0] 기존 Pawn Runtime 초기화 순서를 그대로 실행하고 결과를 Pawn Authority 상태에 기록합니다.
bool UCFVehicleRuntimeComp::InitializeVehicleRuntime(ACFVehiclePawn& VehiclePawn)
{
	// [v1.0.0] 새 초기화 시도에서 차량 코어 Runtime 준비 상태를 먼저 초기화합니다.
	VehiclePawn.bVehicleCoreRuntimeReady = false;

	// [v1.0.0] 새 초기화 시도에서 전투 Runtime 준비 상태를 먼저 초기화합니다.
	VehiclePawn.bVehicleCombatRuntimeReady = false;

	// [v1.0.0] 기존 호환 준비 상태도 차량 코어 상태와 함께 초기화합니다.
	VehiclePawn.bVehicleRuntimeReady = false;
	VehiclePawn.LastVehicleRuntimeSummary = TEXT("VehicleRuntime: InitializeStarted");

	// [v1.0.0] 명시 재초기화에서 이전 출격 탄약 상태가 재사용되지 않도록 Ammo Runtime을 먼저 비웁니다.
	if (VehiclePawn.VehicleAmmoComp)
	{
		VehiclePawn.VehicleAmmoComp->ResetAmmoRuntime();
	}

	// [v1.0.0] VehicleData 자체가 runtime에서 교체될 수 있으므로 새 ChassisMesh를 Layout/WheelSync보다 먼저 SM_Body에 재적용합니다.
	VehiclePawn.ApplyVehicleVisualConfig();
	VehiclePawn.ApplyVehicleDataConfig();

	// [v1.0.0] VehicleData 기반 공통 설정 적용 직후의 요약 문자열입니다.
	const FString DataConfigSummary = VehiclePawn.LastVehicleRuntimeSummary;

	// [v1.0.0] 로컬 Owner 표시 안정화 계층 준비 결과입니다.
	const bool bOwnerVisualReady = VehiclePawn.PrepareOwnerVisualStabilization();

	// [v1.0.0] Owner 표시 루트 재부착 이후 최종 부모 기준으로 레이아웃을 다시 적용합니다.
	VehiclePawn.ApplyVehicleLayoutConfig();

	// [v1.0.0] Owner 표시 루트 재부착 이후 최종 부모 기준으로 터렛 시각 장착을 다시 적용합니다.
	VehiclePawn.ApplyVehicleTurretVisualConfig();

	// [v1.0.0] WheelSync 캡처 직전에 확정된 레이아웃 적용 요약 문자열입니다.
	const FString LayoutConfigSummary = VehiclePawn.LastVehicleRuntimeSummary;

	// [v1.0.0] 차량 입력/물리 Drive 컴포넌트 캐시 준비 결과입니다.
	const bool bDriveReady = (VehiclePawn.VehicleDriveComp != nullptr)
		&& VehiclePawn.VehicleDriveComp->CacheVehicleMovementComponent();

	// [v1.0.0] DataAsset 레이아웃 적용 이후 WheelSync 준비가 성공했는지 여부입니다.
	const bool bWheelSyncReady = VehiclePawn.PrepareWheelSync();

	// [v1.0.0] AimComp가 Owner Pawn과 VehicleCameraComp를 안전하게 찾았는지 여부입니다.
	const bool bAimReady = VehiclePawn.VehicleAimComp
		? VehiclePawn.VehicleAimComp->InitializeAimRuntime()
		: false;

	// [v1.0.0] VehicleData 최대 내구도 또는 안전 기본값으로 차량 내구도가 준비됐는지 여부입니다.
	const bool bHealthReady = VehiclePawn.VehicleHealthComp
		? VehiclePawn.VehicleHealthComp->InitializeFromVehicleData(VehiclePawn.VehicleData)
		: false;

	// [v1.1.0] Fitting Scanner override가 적용되기 전에 차량 자체의 기본 Sensor Source를 먼저 구성합니다.
	// [v1.2.0] VehicleData 기본 Sensor Source 적용 결과이며 invalid explicit SensorData를 조용히 이전 source로 유지하지 않도록 fail-closed합니다.
	bool bVehicleBaseSensorReady = true;
	if (VehiclePawn.VehicleSensorComp)
	{
		// [v1.2.0] 현재 VehicleData가 제공하는 기본 SensorData이며 VehicleData가 없거나 미설정이면 nullptr입니다.
		UCFVehicleSensorData* VehicleBaseSensorData = VehiclePawn.VehicleData
			? VehiclePawn.VehicleData->DefaultSensorData.Get()
			: nullptr;
		bVehicleBaseSensorReady = VehiclePawn.VehicleSensorComp->ApplyVehicleBaseSensorData(VehicleBaseSensorData);
		if (!bVehicleBaseSensorReady)
		{
			VehiclePawn.LastVehicleRuntimeSummary = TEXT("VehicleRuntime: Failed, VehicleBaseSensorDataInvalid");
			return false;
		}
	}

	// [v1.0.0] 기존 활성 프로파일을 Snapshot Weapon 적용 대상으로 유지할 ID입니다.
	const FName RequestedActiveMountProfileId = VehiclePawn.VehicleWeaponComp
		? VehiclePawn.VehicleWeaponComp->GetActiveMountProfileId()
		: NAME_None;

	// [v1.0.0] 첫 BeginPlay는 PreRegister의 Cached 입력을 사용하고, 명시 재초기화는 같은 질량인지 검증한 새 입력만 준비합니다.
	const bool bFittingPrepared = VehiclePawn.VehicleFittingComp
		? (VehiclePawn.VehicleFittingComp->HasPreparedRuntimeInput()
			? true
			: VehiclePawn.VehicleFittingComp->PrepareInitialSortieFitting(
				VehiclePawn.VehicleFittingData,
				VehiclePawn.VehicleData,
				RequestedActiveMountProfileId))
		: false;

	// [v1.0.0] Snapshot 경로는 Configured Mass와 VehicleMesh 실제 질량 검증을 통과해야 하위 Runtime Commit을 허용합니다.
	const bool bInitialMassReady = bFittingPrepared
		&& VehiclePawn.VerifyInitialSortieRuntimeMass();

	// [v1.0.0] 질량 검증을 통과한 같은 Cached Snapshot의 Weapon·Defense 입력만 한 트랜잭션으로 Commit합니다.
	const bool bFittingApplied = bInitialMassReady && VehiclePawn.VehicleFittingComp
		? VehiclePawn.VehicleFittingComp->CommitPreparedSortieFittingToVehicle(
			&VehiclePawn,
			VehiclePawn.VehicleWeaponComp,
			VehiclePawn.VehicleDefenseComp)
		: false;

	// [v1.0.0] Commit 후 실제 Weapon Runtime 준비 상태입니다.
	const bool bWeaponReady = VehiclePawn.VehicleWeaponComp
		&& VehiclePawn.VehicleWeaponComp->IsWeaponRuntimeReady();

	// [v1.0.0] Ammo 기본 서브오브젝트 존재와 finite Snapshot 초기화 결과를 합친 전투 탄약 준비 상태입니다.
	bool bAmmoReady = VehiclePawn.VehicleAmmoComp != nullptr;

	// [v1.0.0] 이번 Applied Snapshot에서 실제 finite Ammo Runtime 초기화가 필요한 무기가 하나라도 있는지 여부입니다.
	bool bFiniteAmmoRuntimeRequired = false;

	if (bFittingApplied
		&& VehiclePawn.VehicleFittingComp
		&& VehiclePawn.VehicleFittingComp->HasAppliedFittingSnapshot())
	{
		// [v1.0.0] Initial Mass와 Weapon·Defense Commit에 사용한 바로 그 Applied Fitting Snapshot 복사본입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehiclePawn.VehicleFittingComp->GetAppliedFittingSnapshot();

		// [v1.0.0] WeaponInstanceId별 독립 장전 상태를 만들 finite 무기 초기화 입력 목록입니다.
		TArray<FCFWeaponAmmoInitialization> WeaponAmmoInitializations;
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			// [v1.0.0] 이 Mount에 Snapshot이 실제 해결한 WeaponData입니다.
			UCFWeaponData* ResolvedWeaponData = ResolvedMount.WeaponData;
			if (!IsValid(ResolvedWeaponData) || ResolvedWeaponData->bUseInfiniteAmmoForDebug)
			{
				continue;
			}

			bFiniteAmmoRuntimeRequired = true;
			if (!ResolvedWeaponData->UsesFiniteAmmoRuntime() || ResolvedMount.MountProfileId.IsNone())
			{
				bAmmoReady = false;
				continue;
			}

			// [v1.0.0] 같은 WeaponData를 여러 Mount에 장착해도 Loaded를 독립 소유하게 할 WeaponInstance 초기화 입력입니다.
			FCFWeaponAmmoInitialization WeaponAmmoInitialization;
			WeaponAmmoInitialization.WeaponInstanceId = ResolvedMount.MountProfileId;
			WeaponAmmoInitialization.WeaponData = ResolvedWeaponData;
			WeaponAmmoInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;
			WeaponAmmoInitializations.Add(WeaponAmmoInitialization);
		}

		// [v1.0.0] 명시적 출격 탄약 또는 finite WeaponInstance가 있으면 VehicleAmmoComp에 실제 Runtime을 구성해야 하는지 여부입니다.
		const bool bShouldInitializeAmmoRuntime = !AppliedFittingSnapshot.InitialSortieAmmoLoads.IsEmpty()
			|| !WeaponAmmoInitializations.IsEmpty();
		if (bAmmoReady && bShouldInitializeAmmoRuntime)
		{
			bAmmoReady = VehiclePawn.VehicleAmmoComp
				&& VehiclePawn.VehicleAmmoComp->InitializeAmmoRuntime(
					&VehiclePawn,
					AppliedFittingSnapshot.InitialSortieAmmoLoads,
					WeaponAmmoInitializations);
		}
		else if (bFiniteAmmoRuntimeRequired)
		{
			bAmmoReady = false;
		}
	}

	// [v1.0.0] Snapshot 장비가 반영된 최종 Weapon 캐시로 터렛 시각화를 다시 적용합니다.
	if (bFittingApplied)
	{
		VehiclePawn.ApplyVehicleTurretVisualConfig();
	}

	// [v1.0.0] LauncherComp를 최종 Weapon Runtime에 연결합니다.
	const bool bLauncherReady = VehiclePawn.LauncherComp
		? VehiclePawn.LauncherComp->InitializeLauncherRuntime(&VehiclePawn, VehiclePawn.VehicleWeaponComp)
		: false;

	// [v1.0.0] Commit 후 실제 DefenseData 초기화 여부입니다.
	const bool bDefenseDataReady = VehiclePawn.VehicleDefenseComp
		&& VehiclePawn.VehicleDefenseComp->IsDefenseInitialized();

	// [v1.0.0] Legacy Fallback 상태여도 정식 방어 진입점을 제공할 컴포넌트 존재 여부입니다.
	const bool bDefenseComponentReady = VehiclePawn.VehicleDefenseComp != nullptr;

	if (VehiclePawn.CombatFxComp)
	{
		VehiclePawn.CombatFxComp->InitializeCombatFxRuntime(
			&VehiclePawn,
			VehiclePawn.VehicleData,
			VehiclePawn.VehicleHealthComp);
	}

	// [v1.0.0] Sensor Foundation은 TargetSelect와 독립 초기화하며 기존 CoreReady/CombatReady의 필수 조건으로 사용하지 않습니다.
	if (VehiclePawn.VehicleSensorComp)
	{
		VehiclePawn.VehicleSensorComp->InitializeSensorRuntime();
	}

	// [v1.0.0] 기본 주행·물리·내구도·피팅을 사용할 수 있는 차량 코어 Runtime 준비 상태입니다.
	VehiclePawn.bVehicleCoreRuntimeReady = bDriveReady
		&& bWheelSyncReady
		&& bHealthReady
		&& bDefenseComponentReady
		&& bFittingApplied;

	// [v1.0.0] 전투 Runtime 준비 판정에 포함할 TargetSelectComp 존재 여부입니다.
	const bool bTargetSelectReady = VehiclePawn.TargetSelectComp != nullptr;

	// [v1.0.0] 차량 코어에 Aim·Weapon·Ammo·Launcher·TargetSelect가 모두 연결된 전투 Runtime 준비 상태입니다.
	VehiclePawn.bVehicleCombatRuntimeReady = VehiclePawn.bVehicleCoreRuntimeReady
		&& bAimReady
		&& bWeaponReady
		&& bAmmoReady
		&& bLauncherReady
		&& bTargetSelectReady;

	// [v1.0.0] 기존 Tick·Debug·Blueprint 호환 값은 차량 코어 Runtime 준비 상태와 동일하게 유지합니다.
	VehiclePawn.bVehicleRuntimeReady = VehiclePawn.bVehicleCoreRuntimeReady;

	// [v1.0.0] 실제 finite Ammo Runtime, 기존 무한탄 호환 또는 초기화 실패를 구분해 표시할 탄약 런타임 상태입니다.
	const TCHAR* AmmoRuntimeState = !VehiclePawn.VehicleAmmoComp
		? TEXT("Missing")
		: (!bAmmoReady
			? TEXT("Failed")
			: (VehiclePawn.VehicleAmmoComp->IsAmmoRuntimeInitialized()
				? TEXT("Ready")
				: TEXT("InfiniteCompatibility")));

	// [v1.0.0] 실제 DefenseData 초기화 또는 Legacy Fallback 상태를 구분해 표시할 방어 런타임 상태입니다.
	const TCHAR* DefenseRuntimeState = !bDefenseComponentReady
		? TEXT("Missing")
		: (bDefenseDataReady ? TEXT("Ready") : TEXT("LegacyFallback"));

	// [v1.0.0] VehicleFittingComp가 기록한 Legacy·Snapshot 적용 결과입니다.
	const FString FittingRuntimeSummary = VehiclePawn.VehicleFittingComp
		? VehiclePawn.VehicleFittingComp->GetLastFittingRuntimeSummary()
		: TEXT("FittingRuntime: ComponentMissing");

	// [v1.0.0] Initial Mass Prepare·실제 VehicleMesh 검증 결과입니다.
	const FString InitialMassSummary = VehiclePawn.VehicleFittingComp
		? VehiclePawn.VehicleFittingComp->GetLastInitialMassSummary()
		: TEXT("InitialMass: ComponentMissing");

	VehiclePawn.LastVehicleRuntimeSummary = FString::Printf(
		TEXT("VehicleRuntime: Data=%s, Fitting=%s, Mass=%s, Drive=%s, WheelSync=%s, Aim=%s, Weapon=%s, Ammo=%s, Launcher=%s, Health=%s, Defense=%s, TargetSelect=%s, OwnerVisual=%s, CoreReady=%s, CombatReady=%s | %s | %s | %s | %s | %s"),
		VehiclePawn.VehicleData ? TEXT("Present") : TEXT("Missing"),
		bFittingApplied ? TEXT("Applied") : TEXT("Failed"),
		bInitialMassReady ? TEXT("Ready") : TEXT("Failed"),
		bDriveReady ? TEXT("Ready") : TEXT("Missing"),
		bWheelSyncReady ? TEXT("Ready") : TEXT("Missing"),
		bAimReady ? TEXT("Ready") : TEXT("Missing"),
		bWeaponReady ? TEXT("Ready") : TEXT("Missing"),
		AmmoRuntimeState,
		bLauncherReady ? TEXT("Ready") : TEXT("Missing"),
		bHealthReady ? TEXT("Ready") : TEXT("Missing"),
		DefenseRuntimeState,
		bTargetSelectReady ? TEXT("Ready") : TEXT("Missing"),
		bOwnerVisualReady ? TEXT("Ready") : TEXT("Skipped"),
		VehiclePawn.bVehicleCoreRuntimeReady ? TEXT("True") : TEXT("False"),
		VehiclePawn.bVehicleCombatRuntimeReady ? TEXT("True") : TEXT("False"),
		*FittingRuntimeSummary,
		*InitialMassSummary,
		*DataConfigSummary,
		*LayoutConfigSummary,
		*VehiclePawn.LastTurretVisualSummary);

	return VehiclePawn.bVehicleRuntimeReady;
}

// [v1.0.0] 이미 Commit된 Fitting 기준 장비 의존 Runtime만 다시 구성합니다.
bool UCFVehicleRuntimeComp::RefreshFittingDependentRuntime(ACFVehiclePawn& VehiclePawn)
{
	// [v1.0.0] Fitting Commit이 완료되어 장비 Runtime 입력을 readback할 수 있는지 여부입니다.
	const bool bFittingRuntimeApplied = VehiclePawn.VehicleFittingComp
		&& VehiclePawn.VehicleFittingComp->HasAppliedRuntimeInput();
	if (!bFittingRuntimeApplied)
	{
		VehiclePawn.bVehicleCombatRuntimeReady = false;
		VehiclePawn.LastVehicleRuntimeSummary = TEXT("FittingDependentRuntime: Failed, AppliedFittingRuntimeMissing");
		return false;
	}

	// [v1.0.0] 이전 장전·예비·예약·Reload 상태가 새 Snapshot에 잔류하지 않게 비울 Ammo Runtime입니다.
	if (VehiclePawn.VehicleAmmoComp)
	{
		VehiclePawn.VehicleAmmoComp->ResetAmmoRuntime();
	}

	// [v1.0.0] Ammo 기본 서브오브젝트 존재와 finite Snapshot 초기화 결과를 합친 탄약 준비 상태입니다.
	bool bAmmoReady = VehiclePawn.VehicleAmmoComp != nullptr;

	// [v1.0.0] 현재 Applied Snapshot에 실제 finite Ammo Runtime이 필요한 무기가 하나라도 있는지 여부입니다.
	bool bFiniteAmmoRuntimeRequired = false;

	// [v1.0.0] 현재 Applied Runtime이 Snapshot 모드인지 여부입니다.
	const bool bHasAppliedFittingSnapshot = VehiclePawn.VehicleFittingComp->HasAppliedFittingSnapshot();
	if (bHasAppliedFittingSnapshot)
	{
		// [v1.0.0] 새 Ammo/Turret/Launcher를 구성할 현재 Applied Fitting Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehiclePawn.VehicleFittingComp->GetAppliedFittingSnapshot();

		// [v1.0.0] WeaponInstanceId별 독립 장전 상태를 만들 finite 무기 초기화 입력입니다.
		TArray<FCFWeaponAmmoInitialization> WeaponAmmoInitializations;
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			// [v1.0.0] 이 Mount에 Snapshot이 실제 해결한 WeaponData입니다.
			UCFWeaponData* ResolvedWeaponData = ResolvedMount.WeaponData;
			if (!IsValid(ResolvedWeaponData) || ResolvedWeaponData->bUseInfiniteAmmoForDebug)
			{
				continue;
			}

			bFiniteAmmoRuntimeRequired = true;
			if (!ResolvedWeaponData->UsesFiniteAmmoRuntime() || ResolvedMount.MountProfileId.IsNone())
			{
				bAmmoReady = false;
				continue;
			}

			// [v1.0.0] 같은 WeaponData를 여러 Mount에 장착해도 Loaded 상태를 독립 소유할 초기화 입력입니다.
			FCFWeaponAmmoInitialization WeaponAmmoInitialization;
			WeaponAmmoInitialization.WeaponInstanceId = ResolvedMount.MountProfileId;
			WeaponAmmoInitialization.WeaponData = ResolvedWeaponData;
			WeaponAmmoInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;
			WeaponAmmoInitializations.Add(WeaponAmmoInitialization);
		}

		// [v1.0.0] 명시적 출격 탄약 또는 finite WeaponInstance 때문에 실제 Ammo Runtime 구성이 필요한지 여부입니다.
		const bool bShouldInitializeAmmoRuntime = !AppliedFittingSnapshot.InitialSortieAmmoLoads.IsEmpty()
			|| !WeaponAmmoInitializations.IsEmpty();
		if (bAmmoReady && bShouldInitializeAmmoRuntime)
		{
			bAmmoReady = VehiclePawn.VehicleAmmoComp
				&& VehiclePawn.VehicleAmmoComp->InitializeAmmoRuntime(
					&VehiclePawn,
					AppliedFittingSnapshot.InitialSortieAmmoLoads,
					WeaponAmmoInitializations);
		}
		else if (bFiniteAmmoRuntimeRequired)
		{
			bAmmoReady = false;
		}
	}

	// [v1.0.0] Snapshot 또는 Legacy 복구 후 현재 Weapon Runtime Source에 맞게 단일 활성 Turret Visual을 재구성합니다.
	VehiclePawn.ApplyVehicleTurretVisualConfig();

	// [v1.0.0] 최종 Weapon Runtime에 Launcher를 다시 연결한 결과입니다.
	const bool bLauncherReady = VehiclePawn.LauncherComp
		? VehiclePawn.LauncherComp->InitializeLauncherRuntime(&VehiclePawn, VehiclePawn.VehicleWeaponComp)
		: false;

	// [v1.0.0] 장비 교체 뒤에도 기존 Aim Runtime이 준비 상태인지 readback합니다.
	const bool bAimReady = VehiclePawn.VehicleAimComp
		&& VehiclePawn.VehicleAimComp->IsAimRuntimeReady();

	// [v1.0.0] Fitting Commit 결과 Weapon Runtime이 준비 상태인지 readback합니다.
	const bool bWeaponReady = VehiclePawn.VehicleWeaponComp
		&& VehiclePawn.VehicleWeaponComp->IsWeaponRuntimeReady();

	// [v1.0.0] 기존 전투 입력 계약에 필요한 TargetSelectComp가 존재하는지 여부입니다.
	const bool bTargetSelectReady = VehiclePawn.TargetSelectComp != nullptr;

	VehiclePawn.bVehicleCombatRuntimeReady = VehiclePawn.bVehicleCoreRuntimeReady
		&& bAimReady
		&& bWeaponReady
		&& bAmmoReady
		&& bLauncherReady
		&& bTargetSelectReady;

	// [v1.0.0] 기존 호환 RuntimeReady는 장비 hot apply에서도 CoreReady와 동일 의미를 유지합니다.
	VehiclePawn.bVehicleRuntimeReady = VehiclePawn.bVehicleCoreRuntimeReady;

	// [v1.0.0] 현재 Ammo 상태를 finite 준비/무한탄 호환/실패로 구분한 bounded readback입니다.
	const TCHAR* AmmoRuntimeState = !VehiclePawn.VehicleAmmoComp
		? TEXT("Missing")
		: (!bAmmoReady
			? TEXT("Failed")
			: (VehiclePawn.VehicleAmmoComp->IsAmmoRuntimeInitialized()
				? TEXT("Ready")
				: TEXT("InfiniteCompatibility")));

	VehiclePawn.LastVehicleRuntimeSummary = FString::Printf(
		TEXT("FittingDependentRuntime: Fitting=%s, Aim=%s, Weapon=%s, Ammo=%s, Launcher=%s, TargetSelect=%s, CoreReady=%s, CombatReady=%s | %s | %s"),
		bHasAppliedFittingSnapshot ? TEXT("Snapshot") : TEXT("Legacy"),
		bAimReady ? TEXT("Ready") : TEXT("Missing"),
		bWeaponReady ? TEXT("Ready") : TEXT("Missing"),
		AmmoRuntimeState,
		bLauncherReady ? TEXT("Ready") : TEXT("Missing"),
		bTargetSelectReady ? TEXT("Ready") : TEXT("Missing"),
		VehiclePawn.bVehicleCoreRuntimeReady ? TEXT("True") : TEXT("False"),
		VehiclePawn.bVehicleCombatRuntimeReady ? TEXT("True") : TEXT("False"),
		VehiclePawn.VehicleFittingComp
			? *VehiclePawn.VehicleFittingComp->GetLastFittingRuntimeSummary()
			: TEXT("FittingRuntime: ComponentMissing"),
		*VehiclePawn.LastTurretVisualSummary);

	return VehiclePawn.bVehicleCombatRuntimeReady;
}

// [v1.0.0] PreRegister Super 호출 전에 Initial Sortie 질량을 준비해 Chaos Movement에 기록합니다.
bool UCFVehicleRuntimeComp::PrepareInitialSortieRuntimeMass(ACFVehiclePawn& VehiclePawn)
{
	if (!VehiclePawn.VehicleFittingComp)
	{
		VehiclePawn.LastVehicleRuntimeSummary = TEXT("InitialMass: VehicleFittingCompMissing");
		return false;
	}

	// [v1.0.0] PreRegister Snapshot의 활성 장착 입력으로 유지할 프로파일 ID입니다.
	const FName RequestedActiveMountProfileId = VehiclePawn.VehicleWeaponComp
		? VehiclePawn.VehicleWeaponComp->GetActiveMountProfileId()
		: NAME_None;
	if (!VehiclePawn.VehicleFittingComp->PrepareInitialSortieFitting(
		VehiclePawn.VehicleFittingData,
		VehiclePawn.VehicleData,
		RequestedActiveMountProfileId))
	{
		VehiclePawn.LastVehicleRuntimeSummary = VehiclePawn.VehicleFittingComp->GetLastInitialMassSummary();
		return false;
	}

	if (!VehiclePawn.VehicleFittingComp->ShouldApplyPreparedInitialMass())
	{
		VehiclePawn.LastVehicleRuntimeSummary = VehiclePawn.VehicleFittingComp->GetLastInitialMassSummary();
		return true;
	}

	// [v1.0.0] 물리 생성 전에 Target Mass를 기록할 실제 Chaos Wheeled Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent =
		VehiclePawn.FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	if (!ResolvedVehicleMovementComponent)
	{
		const bool bFallbackPrepared = VehiclePawn.VehicleFittingComp->FallbackPreparedInitialMassToLegacy(
			VehiclePawn.VehicleData,
			RequestedActiveMountProfileId,
			TEXT("VehicleMovementMissingBeforePhysics"));
		VehiclePawn.LastVehicleRuntimeSummary = VehiclePawn.VehicleFittingComp->GetLastInitialMassSummary();
		return bFallbackPrepared;
	}

	// [v1.0.0] 실패 시 Super 호출 전에 복원할 기존 Chaos Movement Mass입니다.
	const float PreviousMovementMassKg = ResolvedVehicleMovementComponent->Mass;

	// [v1.0.0] Cached Snapshot에서 읽은 초기 출격 Target Mass입니다.
	const float TargetMovementMassKg = VehiclePawn.VehicleFittingComp->GetPreparedInitialMassKg();
	ResolvedVehicleMovementComponent->Mass = TargetMovementMassKg;

	if (!VehiclePawn.VehicleFittingComp->RecordInitialMassBeforePhysics(
		PreviousMovementMassKg,
		ResolvedVehicleMovementComponent->Mass))
	{
		ResolvedVehicleMovementComponent->Mass = PreviousMovementMassKg;
		const bool bFallbackPrepared = VehiclePawn.VehicleFittingComp->FallbackPreparedInitialMassToLegacy(
			VehiclePawn.VehicleData,
			RequestedActiveMountProfileId,
			TEXT("MovementMassRecordFailed"));
		VehiclePawn.LastVehicleRuntimeSummary = VehiclePawn.VehicleFittingComp->GetLastInitialMassSummary();
		return bFallbackPrepared;
	}

	VehiclePawn.LastVehicleRuntimeSummary = VehiclePawn.VehicleFittingComp->GetLastInitialMassSummary();
	return true;
}

// [v1.0.0] BeginPlay Runtime 초기화에서 Initial Sortie 질량과 실제 Physics 상태를 검증합니다.
bool UCFVehicleRuntimeComp::VerifyInitialSortieRuntimeMass(ACFVehiclePawn& VehiclePawn)
{
	if (!VehiclePawn.VehicleFittingComp)
	{
		return false;
	}

	if (VehiclePawn.VehicleFittingComp->UsesLegacyInitialMass())
	{
		return VehiclePawn.VehicleFittingComp->VerifyInitialMassAfterPhysics(
			0.0f,
			0.0f,
			false,
			false,
			false);
	}

	// [v1.0.0] Snapshot Target과 비교할 현재 Chaos Movement 설정값입니다.
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent =
		VehiclePawn.ResolveVehicleMovementComponent(
			TEXT("InitialMass: DriveCompCacheFailed"),
			TEXT("InitialMass: VehicleMovementMissing"));

	// [v1.0.0] 실제 Physics State·PhysicsAsset·Body Mass를 제공할 상속 VehicleMesh입니다.
	USkeletalMeshComponent* VehicleMeshComponent = VehiclePawn.GetMesh();
	if (!ResolvedVehicleMovementComponent || !VehicleMeshComponent)
	{
		return VehiclePawn.VehicleFittingComp->VerifyInitialMassAfterPhysics(
			0.0f,
			0.0f,
			false,
			false,
			false);
	}

	// [v1.0.0] Movement Component에 현재 설정된 Chaos 차량 질량입니다.
	const float ConfiguredMovementMassKg = ResolvedVehicleMovementComponent->Mass;

	// [v1.0.0] Physics State 생성 뒤 VehicleMesh BodyInstance가 보고하는 실제 총질량입니다.
	const float ActualVehicleMeshMassKg = VehicleMeshComponent->GetMass();

	// [v1.0.0] VehicleMesh가 실제 Physics State를 생성했는지 여부입니다.
	const bool bHasVehiclePhysicsState = VehicleMeshComponent->IsPhysicsStateCreated();

	// [v1.0.0] VehicleMesh Root Body가 Chaos 물리 시뮬레이션 중인지 여부입니다.
	const bool bVehicleSimulatesPhysics = VehicleMeshComponent->IsSimulatingPhysics();

	// [v1.0.0] VehicleMesh가 실제 충돌·관성 원본 PhysicsAsset을 해석했는지 여부입니다.
	const bool bHasVehiclePhysicsAsset = VehicleMeshComponent->GetPhysicsAsset() != nullptr;

	return VehiclePawn.VehicleFittingComp->VerifyInitialMassAfterPhysics(
		ConfiguredMovementMassKg,
		ActualVehicleMeshMassKg,
		bHasVehiclePhysicsState,
		bVehicleSimulatesPhysics,
		bHasVehiclePhysicsAsset);
}

// [v1.0.0] VehicleData의 Movement→Reference→WheelPhysics→WheelVisual→TurretVisual→DriveState 적용 순서를 보존합니다.
void UCFVehicleRuntimeComp::ApplyVehicleDataConfig(ACFVehiclePawn& VehiclePawn)
{
	VehiclePawn.ApplyVehicleMovementConfig();
	VehiclePawn.ApplyVehicleReferenceConfig();
	VehiclePawn.ApplyVehicleWheelPhysicsConfig();
	VehiclePawn.ApplyVehicleWheelVisualConfig();
	VehiclePawn.ApplyVehicleTurretVisualConfig();
	if (VehiclePawn.VehicleDriveComp && VehiclePawn.VehicleData)
	{
		VehiclePawn.VehicleDriveComp->ApplyDriveStateConfig(VehiclePawn.VehicleData->DriveStateConfig);
	}
}
