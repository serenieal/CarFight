// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-13
// Description: CF-FQ-031 차량 탄약 Runtime 단일 소유 컴포넌트 구현
// Scope: 출격 실제 총 탄약을 WeaponInstance별 장전량과 AmmoId별 공유 예비량으로 분리하고 SingleCycle Transaction, Launcher 예약, FullMagazine Reload와 HUD Snapshot을 계산합니다.
// Changelog:
// - v1.4.0: AMMO-P0-06 HUD가 Reload 진행률을 추정하지 않도록 Snapshot에 실제 전체 Reload 시간을 기록.
// - v1.3.0: AMMO-P0-05 Reload 중에만 동작하는 Pause-safe Tick, 수동·자동 FullMagazine 시작, 완료 시점 Reserve 재확인·이동, 취소와 Reloading Action Lock을 구현.
// - v1.2.0: AMMO-P0-04 Ripple·Salvo 전체 유효 발수 사전 예약, 성공 발사별 Commit, 실패 발사별 Release, Terminal 전체 반환과 LauncherSequenceActive Action Lock을 구현.
// - v1.1.0: AMMO-P0-03 SingleCycle Validate·Reserve·Commit·Rollback과 Transaction 예약량을 ImmediateUsable 계산에 연결.
// - v1.0.0: AMMO-P0-02 원자 초기화, 공유 Reserve, 독립 Loaded, Snapshot, 반복 초기화와 Reset 구현.
// Migration:
// - MaximumLoadableAmmoCount는 이 초기화 경로의 현재 수량 입력으로 사용하지 않습니다.
// - SingleCycle 실행 실패는 예약만 반환하고 실제 LoadedAmmoCount를 소비하지 않습니다.
// - Reload 시작 시 수량을 이동하지 않고 완료 순간의 실제 공유 Reserve를 다시 확인해 이동합니다.
// - Reload Tick은 bTickEvenWhenPaused=false이므로 게임 Pause 동안 진행되지 않습니다.

#include "CFVehicleAmmoComp.h"

#include "CFAmmoData.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

// [v1.3.0] Reload 중에만 일반 게임 Tick을 사용하도록 탄약 컴포넌트 기본값을 설정합니다.
UCFVehicleAmmoComp::UCFVehicleAmmoComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
}

// [v1.3.0] 활성 Reload의 경과 시간을 게임 DeltaSeconds로 진행하고 완료 시 Reserve→Loaded 이동을 확정합니다.
void UCFVehicleAmmoComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAmmoRuntimeInitialized)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// [v1.3.0] 파괴된 차량에서는 진행 중 Reload를 수량 이동 없이 취소하고 무기 행동을 파괴 사유로 잠급니다.
	const UCFVehicleHealthComp* VehicleHealthComp = IsValid(OwnerVehiclePawn)
		? OwnerVehiclePawn->GetVehicleHealthComp()
		: nullptr;
	const bool bOwnerVehicleDestroyed = VehicleHealthComp && VehicleHealthComp->IsDestroyed();

	// [v1.3.0] 유효한 게임 DeltaSeconds만 Reload 경과 시간에 반영합니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime) ? FMath::Max(DeltaTime, 0.0f) : 0.0f;
	for (TPair<FName, FCFWeaponAmmoRuntime>& WeaponAmmoPair : WeaponAmmoRuntimeByInstanceId)
	{
		FCFWeaponAmmoRuntime& WeaponAmmoRuntime = WeaponAmmoPair.Value;
		if (WeaponAmmoRuntime.ReloadState != ECFWeaponReloadState::Reloading)
		{
			continue;
		}

		if (bOwnerVehicleDestroyed)
		{
			CancelReloadInternal(
				WeaponAmmoRuntime,
				ECFWeaponReloadState::Disabled,
				ECFWeaponActionLockReason::VehicleDestroyed);
			OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(WeaponAmmoRuntime));
			continue;
		}

		if (!IsValid(WeaponAmmoRuntime.WeaponData) || !IsValid(WeaponAmmoRuntime.CurrentAmmoData))
		{
			CancelReloadInternal(
				WeaponAmmoRuntime,
				ECFWeaponReloadState::NotInitialized,
				ECFWeaponActionLockReason::None);
			OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(WeaponAmmoRuntime));
			continue;
		}

		WeaponAmmoRuntime.ReloadElapsedSeconds = FMath::Min(
			WeaponAmmoRuntime.ReloadElapsedSeconds + SafeDeltaTime,
			WeaponAmmoRuntime.ReloadDurationSeconds);
		if (WeaponAmmoRuntime.ReloadElapsedSeconds + KINDA_SMALL_NUMBER >= WeaponAmmoRuntime.ReloadDurationSeconds)
		{
			CompleteReloadInternal(WeaponAmmoRuntime);
		}
		else
		{
			OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(WeaponAmmoRuntime));
		}
	}

	RefreshReloadTickEnabled();
}

// [v1.0.0] 월드 종료에서 출격 탄약 상태가 다음 수명으로 남지 않도록 초기화합니다.
void UCFVehicleAmmoComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetAmmoRuntime();
	Super::EndPlay(EndPlayReason);
}

// [v1.0.0] 탄종별 실제 출격 적재량과 무기 인스턴스별 초기 장전량으로 Runtime을 원자적으로 구성합니다.
bool UCFVehicleAmmoComp::InitializeAmmoRuntime(
	ACFVehiclePawn* InOwnerVehiclePawn,
	const TArray<FCFAmmoSortieLoad>& SortieAmmoLoads,
	const TArray<FCFWeaponAmmoInitialization>& WeaponInitializations)
{
	if (!IsValid(InOwnerVehiclePawn))
	{
		LastAmmoRuntimeSummary = TEXT("AmmoRuntime: InitializeFailed, Reason=InvalidOwner");
		return false;
	}

	// [v1.0.0] 검증 완료 전 기존 Runtime을 건드리지 않기 위한 임시 출격 전체 탄약 Map입니다.
	TMap<FName, int32> InitialSortieAmmoCountByAmmoId;
	for (const FCFAmmoSortieLoad& SortieAmmoLoad : SortieAmmoLoads)
	{
		UCFAmmoData* AmmoData = SortieAmmoLoad.AmmoData;
		if (!IsValid(AmmoData)
			|| !AmmoData->IsAmmoDataValid()
			|| SortieAmmoLoad.InitialSortieAmmoCount < 0
			|| InitialSortieAmmoCountByAmmoId.Contains(AmmoData->AmmoId))
		{
			LastAmmoRuntimeSummary = TEXT("AmmoRuntime: InitializeFailed, Reason=InvalidOrDuplicateSortieAmmo");
			return false;
		}

		InitialSortieAmmoCountByAmmoId.Add(AmmoData->AmmoId, SortieAmmoLoad.InitialSortieAmmoCount);
	}

	// [v1.0.0] 검증 완료 전 기존 Runtime을 건드리지 않기 위한 임시 WeaponInstance 상태 Map입니다.
	TMap<FName, FCFWeaponAmmoRuntime> PendingWeaponAmmoRuntimeByInstanceId;
	// [v1.0.0] 같은 AmmoId를 공유하는 여러 무기의 초기 장전량 합계입니다.
	TMap<FName, int32> InitialLoadedAmmoCountByAmmoId;

	for (const FCFWeaponAmmoInitialization& WeaponInitialization : WeaponInitializations)
	{
		UCFWeaponData* WeaponData = WeaponInitialization.WeaponData;
		if (WeaponInitialization.WeaponInstanceId.IsNone()
			|| !IsValid(WeaponData)
			|| PendingWeaponAmmoRuntimeByInstanceId.Contains(WeaponInitialization.WeaponInstanceId))
		{
			LastAmmoRuntimeSummary = TEXT("AmmoRuntime: InitializeFailed, Reason=InvalidOrDuplicateWeaponInstance");
			return false;
		}

		if (!WeaponData->UsesFiniteAmmoRuntime() || !IsValid(WeaponData->DefaultAmmoData))
		{
			LastAmmoRuntimeSummary = FString::Printf(
				TEXT("AmmoRuntime: InitializeFailed, Reason=WeaponNotFinite, WeaponInstance=%s"),
				*WeaponInitialization.WeaponInstanceId.ToString());
			return false;
		}

		UCFAmmoData* AmmoData = WeaponData->DefaultAmmoData;
		if (!AmmoData->IsAmmoDataValid() || !InitialSortieAmmoCountByAmmoId.Contains(AmmoData->AmmoId))
		{
			LastAmmoRuntimeSummary = FString::Printf(
				TEXT("AmmoRuntime: InitializeFailed, Reason=MissingSortieAmmo, WeaponInstance=%s"),
				*WeaponInitialization.WeaponInstanceId.ToString());
			return false;
		}

		// [v1.0.0] WeaponData 기본값 또는 명시 Override에서 얻은 실제 출격 초기 장전량입니다.
		const int32 RequestedInitialLoadedAmmoCount = WeaponInitialization.InitialLoadedAmmoCountOverride >= 0
			? WeaponInitialization.InitialLoadedAmmoCountOverride
			: WeaponData->GetEffectiveInitialLoadedAmmoCount();
		// [v1.0.0] 현재 무기의 유효 탄창 최대 용량입니다.
		const int32 MagazineCapacity = WeaponData->GetEffectiveMagazineCapacity();
		if (RequestedInitialLoadedAmmoCount < 0 || RequestedInitialLoadedAmmoCount > MagazineCapacity)
		{
			LastAmmoRuntimeSummary = FString::Printf(
				TEXT("AmmoRuntime: InitializeFailed, Reason=InitialLoadedOutOfRange, WeaponInstance=%s, Loaded=%d, Capacity=%d"),
				*WeaponInitialization.WeaponInstanceId.ToString(),
				RequestedInitialLoadedAmmoCount,
				MagazineCapacity);
			return false;
		}

		// [v1.0.0] 검증이 끝나면 Commit할 WeaponInstance별 초기 Runtime입니다.
		FCFWeaponAmmoRuntime WeaponAmmoRuntime;
		WeaponAmmoRuntime.WeaponInstanceId = WeaponInitialization.WeaponInstanceId;
		WeaponAmmoRuntime.WeaponData = WeaponData;
		WeaponAmmoRuntime.CurrentAmmoData = AmmoData;
		WeaponAmmoRuntime.MagazineCapacity = MagazineCapacity;
		WeaponAmmoRuntime.LoadedAmmoCount = RequestedInitialLoadedAmmoCount;
		WeaponAmmoRuntime.ReservedSequenceAmmoCount = 0;
		WeaponAmmoRuntime.ReloadState = RequestedInitialLoadedAmmoCount > 0
			? ECFWeaponReloadState::Ready
			: ECFWeaponReloadState::Empty;
		WeaponAmmoRuntime.ReloadDurationSeconds = WeaponData->GetEffectiveReloadTimeSeconds();
		PendingWeaponAmmoRuntimeByInstanceId.Add(WeaponInitialization.WeaponInstanceId, WeaponAmmoRuntime);

		InitialLoadedAmmoCountByAmmoId.FindOrAdd(AmmoData->AmmoId) += RequestedInitialLoadedAmmoCount;
	}

	// [v1.0.0] 검증 완료 후 Commit할 탄종별 공유 예비 탄약 수량입니다.
	TMap<FName, int32> PendingReserveAmmoCountByAmmoId;
	for (const TPair<FName, int32>& SortieAmmoPair : InitialSortieAmmoCountByAmmoId)
	{
		// [v1.0.0] 같은 탄종을 사용하는 모든 WeaponInstance에 이미 장전된 탄약 합계입니다.
		const int32 LoadedAmmoCountForAmmoId = InitialLoadedAmmoCountByAmmoId.FindRef(SortieAmmoPair.Key);
		if (LoadedAmmoCountForAmmoId > SortieAmmoPair.Value)
		{
			LastAmmoRuntimeSummary = FString::Printf(
				TEXT("AmmoRuntime: InitializeFailed, Reason=LoadedExceedsSortieTotal, AmmoId=%s, Loaded=%d, Sortie=%d"),
				*SortieAmmoPair.Key.ToString(),
				LoadedAmmoCountForAmmoId,
				SortieAmmoPair.Value);
			return false;
		}

		PendingReserveAmmoCountByAmmoId.Add(SortieAmmoPair.Key, SortieAmmoPair.Value - LoadedAmmoCountForAmmoId);
	}

	OwnerVehiclePawn = InOwnerVehiclePawn;
		ReserveAmmoCountByAmmoId = MoveTemp(PendingReserveAmmoCountByAmmoId);
	WeaponAmmoRuntimeByInstanceId = MoveTemp(PendingWeaponAmmoRuntimeByInstanceId);
	bAmmoRuntimeInitialized = true;

	// [v1.1.0] 최종 Reserve가 확정된 뒤 각 무기의 Ready·Empty·NoReserveAmmo 초기 상태를 다시 계산합니다.
	for (TPair<FName, FCFWeaponAmmoRuntime>& WeaponAmmoPair : WeaponAmmoRuntimeByInstanceId)
	{
		UpdateWeaponReloadStateAfterAmmoChange(WeaponAmmoPair.Value);
	}
	LastAmmoRuntimeSummary = FString::Printf(
		TEXT("AmmoRuntime: Ready, WeaponInstances=%d, AmmoTypes=%d"),
		WeaponAmmoRuntimeByInstanceId.Num(),
		ReserveAmmoCountByAmmoId.Num());
	BroadcastAllAmmoSnapshots();
	return true;
}

// [v1.0.0] 현재 WeaponComp의 활성 무기 하나를 명시적 출격 총 탄수로 초기화하는 단일 무기 편의 경로입니다.
bool UCFVehicleAmmoComp::InitializeActiveWeaponAmmoRuntime(
	ACFVehiclePawn* InOwnerVehiclePawn,
	UCFVehicleWeaponComp* InVehicleWeaponComp,
	const int32 InitialSortieAmmoCount)
{
	if (!IsValid(InOwnerVehiclePawn) || !IsValid(InVehicleWeaponComp) || InitialSortieAmmoCount < 0)
	{
		LastAmmoRuntimeSummary = TEXT("AmmoRuntime: InitializeActiveFailed, Reason=InvalidInput");
		return false;
	}

	UCFWeaponData* ActiveWeaponData = InVehicleWeaponComp->GetActiveWeaponData();
	if (!IsValid(ActiveWeaponData) || !ActiveWeaponData->UsesFiniteAmmoRuntime())
	{
		ResetAmmoRuntime();
		OwnerVehiclePawn = InOwnerVehiclePawn;
		bAmmoRuntimeInitialized = true;
		LastAmmoRuntimeSummary = TEXT("AmmoRuntime: InfiniteCompatibility, FiniteWeaponInstances=0");
		return true;
	}

	FCFAmmoSortieLoad SortieAmmoLoad;
	SortieAmmoLoad.AmmoData = ActiveWeaponData->DefaultAmmoData;
	SortieAmmoLoad.InitialSortieAmmoCount = InitialSortieAmmoCount;

	FCFWeaponAmmoInitialization WeaponInitialization;
	WeaponInitialization.WeaponInstanceId = InVehicleWeaponComp->GetActiveMountProfileId();
	WeaponInitialization.WeaponData = ActiveWeaponData;
	WeaponInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;

	TArray<FCFAmmoSortieLoad> SortieAmmoLoads;
	SortieAmmoLoads.Add(SortieAmmoLoad);
	TArray<FCFWeaponAmmoInitialization> WeaponInitializations;
	WeaponInitializations.Add(WeaponInitialization);
	return InitializeAmmoRuntime(InOwnerVehiclePawn, SortieAmmoLoads, WeaponInitializations);
}

// [v1.0.0] 모든 장전·예비·예약·통계 상태를 비우고 NotInitialized 상태로 되돌립니다.
void UCFVehicleAmmoComp::ResetAmmoRuntime()
{
	SetComponentTickEnabled(false);
	OwnerVehiclePawn = nullptr;
	ReserveAmmoCountByAmmoId.Reset();
	WeaponAmmoRuntimeByInstanceId.Reset();
	bAmmoRuntimeInitialized = false;
	LastAmmoRuntimeSummary = TEXT("AmmoRuntime: NotInitialized");
}

// [v1.3.0] 수동·자동 공통 FullMagazine Reload 시작 검증과 Pending 상태 설정을 수행합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::StartReloadInternal(
	const FName WeaponInstanceId,
	const bool bAutomaticRequest)
{
	if (!bAmmoRuntimeInitialized || WeaponInstanceId.IsNone())
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	// [v1.3.0] Reload를 시작할 대상 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime || !IsValid(WeaponAmmoRuntime->WeaponData))
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	if (!IsValid(WeaponAmmoRuntime->CurrentAmmoData))
	{
		return ECFAmmoTransactionResult::MissingAmmoData;
	}

	// [v1.3.0] 파괴 상태에서 새 Reload를 시작하지 않도록 확인할 차량 내구도 컴포넌트입니다.
	const UCFVehicleHealthComp* VehicleHealthComp = IsValid(OwnerVehiclePawn)
		? OwnerVehiclePawn->GetVehicleHealthComp()
		: nullptr;
	if (VehicleHealthComp && VehicleHealthComp->IsDestroyed())
	{
		return ECFAmmoTransactionResult::ActionLocked;
	}
	if (WeaponAmmoRuntime->ReloadState == ECFWeaponReloadState::Reloading)
	{
		return ECFAmmoTransactionResult::Reloading;
	}
	if (WeaponAmmoRuntime->bWeaponActionLocked
		|| WeaponAmmoRuntime->ReservedSequenceAmmoCount > 0
		|| WeaponAmmoRuntime->ReservedFireTransactionAmmoCount > 0)
	{
		return ECFAmmoTransactionResult::ActionLocked;
	}
	if (WeaponAmmoRuntime->WeaponData->ReloadMode != ECFWeaponReloadMode::FullMagazine)
	{
		return ECFAmmoTransactionResult::ExecutionFailed;
	}

	// [v1.3.0] 현재 무기의 유효 탄창 최대 용량입니다.
	const int32 MagazineCapacity = FMath::Max(WeaponAmmoRuntime->MagazineCapacity, 0);

	// [v1.3.0] 현재 탄창을 최대 용량까지 채우기 위해 필요한 탄약 단위 수입니다.
	const int32 NeededReloadAmount = FMath::Max(MagazineCapacity - WeaponAmmoRuntime->LoadedAmmoCount, 0);
	if (NeededReloadAmount <= 0)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	// [v1.3.0] Reload 시작 시점의 같은 탄종 차량 공유 예비 탄약량입니다.
	const int32 AvailableReserveAmmoCount = GetReserveAmmoCount(WeaponAmmoRuntime->CurrentAmmoData->AmmoId);
	if (AvailableReserveAmmoCount <= 0)
	{
		WeaponAmmoRuntime->ReloadState = ECFWeaponReloadState::NoReserveAmmo;
		return ECFAmmoTransactionResult::NotEnoughLoadedAmmo;
	}
	if (!WeaponAmmoRuntime->WeaponData->bAllowPartialReload
		&& AvailableReserveAmmoCount < NeededReloadAmount)
	{
		return ECFAmmoTransactionResult::NotEnoughLoadedAmmo;
	}

	// [v1.3.0] 완료 순간 이동할 최대 예정 수량이며 실제 공유 Reserve는 완료 시 다시 검증합니다.
	const int32 PendingReloadAmount = WeaponAmmoRuntime->WeaponData->bAllowPartialReload
		? FMath::Min(NeededReloadAmount, AvailableReserveAmmoCount)
		: NeededReloadAmount;
	if (PendingReloadAmount <= 0)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	WeaponAmmoRuntime->ReloadElapsedSeconds = 0.0f;
	WeaponAmmoRuntime->ReloadDurationSeconds = WeaponAmmoRuntime->WeaponData->GetEffectiveReloadTimeSeconds();
	WeaponAmmoRuntime->PendingReloadAmount = PendingReloadAmount;
	WeaponAmmoRuntime->ReloadState = ECFWeaponReloadState::Reloading;
	WeaponAmmoRuntime->bWeaponActionLocked = true;
	WeaponAmmoRuntime->WeaponActionLockReason = ECFWeaponActionLockReason::Reloading;
	LastAmmoRuntimeSummary = FString::Printf(
		TEXT("AmmoRuntime: ReloadStarted, Weapon=%s, Automatic=%s, Pending=%d, Duration=%.3fs"),
		*WeaponInstanceId.ToString(),
		bAutomaticRequest ? TEXT("True") : TEXT("False"),
		PendingReloadAmount,
		WeaponAmmoRuntime->ReloadDurationSeconds);
	SetComponentTickEnabled(true);
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.3.0] Empty 상태와 WeaponData 자동 재장전 정책을 만족하면 Action Lock을 확인한 뒤 Reload를 시작합니다.
bool UCFVehicleAmmoComp::TryStartAutoReloadInternal(FCFWeaponAmmoRuntime& WeaponAmmoRuntime)
{
	if (!IsValid(WeaponAmmoRuntime.WeaponData)
		|| !WeaponAmmoRuntime.WeaponData->bAutoReloadWhenEmpty
		|| WeaponAmmoRuntime.ReloadState == ECFWeaponReloadState::Reloading
		|| WeaponAmmoRuntime.ReloadState == ECFWeaponReloadState::Disabled
		|| WeaponAmmoRuntime.bWeaponActionLocked
		|| WeaponAmmoRuntime.ReservedSequenceAmmoCount > 0
		|| WeaponAmmoRuntime.ReservedFireTransactionAmmoCount > 0)
	{
		return false;
	}

	// [v1.3.0] 자동 Reload가 필요한지 판단할 현재 한 발 최소 탄약 단위 수입니다.
	const int32 AmmoUnitsPerShot = WeaponAmmoRuntime.WeaponData->GetEffectiveAmmoUnitsPerShot();
	if (WeaponAmmoRuntime.LoadedAmmoCount >= AmmoUnitsPerShot)
	{
		return false;
	}

	return StartReloadInternal(WeaponAmmoRuntime.WeaponInstanceId, true) == ECFAmmoTransactionResult::Accepted;
}

// [v1.3.0] Reload 완료 시점의 실제 공유 Reserve와 탄창 부족량을 다시 확인해 수량 이동을 한 번만 확정합니다.
void UCFVehicleAmmoComp::CompleteReloadInternal(FCFWeaponAmmoRuntime& WeaponAmmoRuntime)
{
	if (!IsValid(WeaponAmmoRuntime.CurrentAmmoData))
	{
		CancelReloadInternal(
			WeaponAmmoRuntime,
			ECFWeaponReloadState::NotInitialized,
			ECFWeaponActionLockReason::None);
		OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(WeaponAmmoRuntime));
		return;
	}

	// [v1.3.0] 완료 순간 실제 탄창 빈 공간입니다.
	const int32 NeededReloadAmount = FMath::Max(
		WeaponAmmoRuntime.MagazineCapacity - WeaponAmmoRuntime.LoadedAmmoCount,
		0);

	// [v1.3.0] 완료 순간 다른 WeaponInstance가 이미 사용했을 수 있는 같은 탄종 공유 Reserve의 실제 남은 수량입니다.
	const int32 AvailableReserveAmmoCount = GetReserveAmmoCount(WeaponAmmoRuntime.CurrentAmmoData->AmmoId);

	// [v1.3.0] 시작 때 예정한 양, 완료 순간 탄창 부족량, 실제 공유 Reserve 중 가장 작은 최종 이동 수량입니다.
	const int32 ActualReloadAmount = FMath::Min3(
		FMath::Max(WeaponAmmoRuntime.PendingReloadAmount, 0),
		NeededReloadAmount,
		AvailableReserveAmmoCount);
	if (ActualReloadAmount > 0)
	{
		ReserveAmmoCountByAmmoId.FindOrAdd(WeaponAmmoRuntime.CurrentAmmoData->AmmoId) -= ActualReloadAmount;
		WeaponAmmoRuntime.LoadedAmmoCount += ActualReloadAmount;
		WeaponAmmoRuntime.ReloadedAmmoCountThisSortie += ActualReloadAmount;
	}

	WeaponAmmoRuntime.ReloadElapsedSeconds = 0.0f;
	WeaponAmmoRuntime.PendingReloadAmount = 0;
	WeaponAmmoRuntime.bWeaponActionLocked = false;
	WeaponAmmoRuntime.WeaponActionLockReason = ECFWeaponActionLockReason::None;
	WeaponAmmoRuntime.ReloadState = ECFWeaponReloadState::NotInitialized;
	UpdateWeaponReloadStateAfterAmmoChange(WeaponAmmoRuntime);
	LastAmmoRuntimeSummary = FString::Printf(
		TEXT("AmmoRuntime: ReloadCompleted, Weapon=%s, Moved=%d, Loaded=%d, Reserve=%d"),
		*WeaponAmmoRuntime.WeaponInstanceId.ToString(),
		ActualReloadAmount,
		WeaponAmmoRuntime.LoadedAmmoCount,
		GetReserveAmmoCount(WeaponAmmoRuntime.CurrentAmmoData->AmmoId));
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(WeaponAmmoRuntime));
}

// [v1.3.0] 진행 중 Reload를 수량 이동 없이 정리하고 지정 후속 상태·Action Lock을 적용합니다.
void UCFVehicleAmmoComp::CancelReloadInternal(
	FCFWeaponAmmoRuntime& WeaponAmmoRuntime,
	const ECFWeaponReloadState ResultState,
	const ECFWeaponActionLockReason ResultLockReason)
{
	WeaponAmmoRuntime.ReloadElapsedSeconds = 0.0f;
	WeaponAmmoRuntime.PendingReloadAmount = 0;
	WeaponAmmoRuntime.ReloadState = ResultState;
	WeaponAmmoRuntime.bWeaponActionLocked = ResultLockReason != ECFWeaponActionLockReason::None;
	WeaponAmmoRuntime.WeaponActionLockReason = ResultLockReason;
	if (ResultState == ECFWeaponReloadState::NotInitialized
		&& ResultLockReason == ECFWeaponActionLockReason::None)
	{
		UpdateWeaponReloadStateAfterAmmoChange(WeaponAmmoRuntime);
	}
}

// [v1.3.0] 현재 하나라도 Reloading 상태가 있으면 Tick을 켜고 없으면 다시 끕니다.
void UCFVehicleAmmoComp::RefreshReloadTickEnabled()
{
	bool bHasActiveReload = false;
	for (const TPair<FName, FCFWeaponAmmoRuntime>& WeaponAmmoPair : WeaponAmmoRuntimeByInstanceId)
	{
		if (WeaponAmmoPair.Value.ReloadState == ECFWeaponReloadState::Reloading)
		{
			bHasActiveReload = true;
			break;
		}
	}
	SetComponentTickEnabled(bHasActiveReload);
}

// [v1.3.0] 지정 WeaponInstanceId의 FullMagazine 재장전을 수동으로 요청합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::RequestReload(const FName WeaponInstanceId)
{
	return StartReloadInternal(WeaponInstanceId, false);
}

// [v1.3.0] 현재 VehicleWeaponComp 활성 MountProfileId의 FullMagazine 재장전을 요청합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::RequestActiveWeaponReload(const UCFVehicleWeaponComp* VehicleWeaponComp)
{
	if (!VehicleWeaponComp)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	return RequestReload(VehicleWeaponComp->GetActiveMountProfileId());
}

// [v1.3.0] 진행 중인 지정 무기 Reload를 수량 이동 없이 취소합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::CancelReload(const FName WeaponInstanceId)
{
	// [v1.3.0] 수동 취소 대상 Reload Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	if (WeaponAmmoRuntime->ReloadState != ECFWeaponReloadState::Reloading)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	CancelReloadInternal(
		*WeaponAmmoRuntime,
		ECFWeaponReloadState::NotInitialized,
		ECFWeaponActionLockReason::None);
	LastAmmoRuntimeSummary = FString::Printf(
		TEXT("AmmoRuntime: ReloadCancelled, Weapon=%s"),
		*WeaponInstanceId.ToString());
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	RefreshReloadTickEnabled();
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.3.0] 지정 WeaponInstanceId가 현재 Reloading 상태인지 반환합니다.
bool UCFVehicleAmmoComp::IsReloadActive(const FName WeaponInstanceId) const
{
	// [v1.3.0] Reload 진행 여부를 확인할 WeaponInstance Runtime입니다.
	const FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	return WeaponAmmoRuntime && WeaponAmmoRuntime->ReloadState == ECFWeaponReloadState::Reloading;
}

// [v1.1.0] 상태를 변경하지 않고 SingleCycle 한 발의 현재 탄약 Transaction 가능 여부를 계산합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::EvaluateSingleFireAmmo(const FName WeaponInstanceId) const
{
	if (!bAmmoRuntimeInitialized || WeaponInstanceId.IsNone())
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	// [v1.1.0] 검증 대상 WeaponInstanceId의 실제 탄약 Runtime입니다.
	const FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime || !IsValid(WeaponAmmoRuntime->WeaponData))
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	if (!IsValid(WeaponAmmoRuntime->CurrentAmmoData))
	{
		return ECFAmmoTransactionResult::MissingAmmoData;
	}
	if (WeaponAmmoRuntime->ReloadState == ECFWeaponReloadState::Reloading)
	{
		return ECFAmmoTransactionResult::Reloading;
	}
	if (WeaponAmmoRuntime->bWeaponActionLocked)
	{
		return ECFAmmoTransactionResult::ActionLocked;
	}
	if (WeaponAmmoRuntime->ReservedFireTransactionAmmoCount > 0)
	{
		return ECFAmmoTransactionResult::ActionLocked;
	}

	// [v1.1.0] 한 번의 정상 SingleCycle 발사에 필요한 탄약 단위 수입니다.
	const int32 RequiredAmmoUnits = WeaponAmmoRuntime->WeaponData->GetEffectiveAmmoUnitsPerShot();

	// [v1.1.0] Launcher Sequence 예약과 진행 중 단발 Transaction 예약을 제외한 실제 자유 장전량입니다.
	const int32 ImmediateFreeLoadedAmmoCount = FMath::Max(
		WeaponAmmoRuntime->LoadedAmmoCount
			- WeaponAmmoRuntime->ReservedSequenceAmmoCount
			- WeaponAmmoRuntime->ReservedFireTransactionAmmoCount,
		0);
	return ImmediateFreeLoadedAmmoCount >= RequiredAmmoUnits
		? ECFAmmoTransactionResult::Accepted
		: ECFAmmoTransactionResult::NotEnoughLoadedAmmo;
}

// [v1.1.0] SingleCycle 한 발을 실행하기 전에 현재 장전량·Reload·Action Lock 상태를 검증합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::ValidateSingleFireAmmo(const FName WeaponInstanceId)
{
	// [v1.1.0] 현재 상태를 변경하지 않고 계산한 단발 탄약 검증 결과입니다.
	const ECFAmmoTransactionResult ValidationResult = EvaluateSingleFireAmmo(WeaponInstanceId);
	if (ValidationResult == ECFAmmoTransactionResult::NotEnoughLoadedAmmo)
	{
		if (FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId))
		{
			++WeaponAmmoRuntime->NoAmmoRejectCount;
		}
	}
	return ValidationResult;
}

// [v1.1.0] 실제 SingleCycle 실행 전에 한 발에 필요한 탄약을 장전량에서 임시 예약합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::ReserveSingleFireAmmo(const FName WeaponInstanceId)
{
	// [v1.1.0] 예약 직전 다시 확인한 단발 탄약 상태입니다.
	const ECFAmmoTransactionResult ReservationValidationResult = EvaluateSingleFireAmmo(WeaponInstanceId);
	if (ReservationValidationResult != ECFAmmoTransactionResult::Accepted)
	{
		return ReservationValidationResult;
	}

	// [v1.1.0] 실제 예약량을 기록할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime || !WeaponAmmoRuntime->WeaponData)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	WeaponAmmoRuntime->ReservedFireTransactionAmmoCount = WeaponAmmoRuntime->WeaponData->GetEffectiveAmmoUnitsPerShot();
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.1.0] 성공한 SingleCycle 실행의 임시 예약량을 실제 장전 탄약에서 정확히 한 번 소비합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::CommitSingleFireAmmo(const FName WeaponInstanceId)
{
	// [v1.1.0] 성공 실행 뒤 예약량을 실제 소비할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	// [v1.1.0] 이번 성공 발사에 예약되어 실제로 소비할 탄약 단위 수입니다.
	const int32 ReservedAmmoUnits = WeaponAmmoRuntime->ReservedFireTransactionAmmoCount;
	if (ReservedAmmoUnits <= 0 || ReservedAmmoUnits > WeaponAmmoRuntime->LoadedAmmoCount)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

		WeaponAmmoRuntime->LoadedAmmoCount -= ReservedAmmoUnits;
	WeaponAmmoRuntime->ReservedFireTransactionAmmoCount = 0;
	WeaponAmmoRuntime->FiredAmmoCountThisSortie += ReservedAmmoUnits;
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	if (!TryStartAutoReloadInternal(*WeaponAmmoRuntime))
	{
		OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	}
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.1.0] 실패한 SingleCycle 실행의 임시 예약량을 소비 없이 전부 반환합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::RollbackSingleFireAmmo(const FName WeaponInstanceId)
{
	// [v1.1.0] 실행 실패 뒤 예약량만 반환할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	if (WeaponAmmoRuntime->ReservedFireTransactionAmmoCount <= 0)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	WeaponAmmoRuntime->ReservedFireTransactionAmmoCount = 0;
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.2.0] Ripple·Salvo 시작 전에 현재 자유 장전량으로 실행 가능한 전체 유효 발수를 한 번에 예약하고 Action Lock을 겁니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::ReserveLauncherSequenceAmmo(
	const FName WeaponInstanceId,
	const int32 RequestedShotCount,
	const bool bAllowPartialSequence,
	int32& OutReservedShotCount)
{
	OutReservedShotCount = 0;
	if (!bAmmoRuntimeInitialized || WeaponInstanceId.IsNone())
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	if (RequestedShotCount <= 0)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	// [v1.2.0] Launcher Sequence 전체 예약을 기록할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime || !IsValid(WeaponAmmoRuntime->WeaponData))
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}
	if (!IsValid(WeaponAmmoRuntime->CurrentAmmoData))
	{
		return ECFAmmoTransactionResult::MissingAmmoData;
	}
	if (WeaponAmmoRuntime->ReloadState == ECFWeaponReloadState::Reloading)
	{
		return ECFAmmoTransactionResult::Reloading;
	}
	if (WeaponAmmoRuntime->ReservedSequenceAmmoCount > 0
		|| (WeaponAmmoRuntime->bWeaponActionLocked
			&& WeaponAmmoRuntime->WeaponActionLockReason == ECFWeaponActionLockReason::LauncherSequenceActive))
	{
		return ECFAmmoTransactionResult::SequenceAlreadyActive;
	}
	if (WeaponAmmoRuntime->bWeaponActionLocked || WeaponAmmoRuntime->ReservedFireTransactionAmmoCount > 0)
	{
		return ECFAmmoTransactionResult::ActionLocked;
	}

	// [v1.2.0] 한 Launcher 내부 발사에 필요한 탄약 단위 수입니다.
	const int32 AmmoUnitsPerShot = WeaponAmmoRuntime->WeaponData->GetEffectiveAmmoUnitsPerShot();

	// [v1.2.0] 다른 예약을 제외하고 이번 Sequence 전체 예약에 사용할 수 있는 현재 장전 탄약 단위 수입니다.
	const int32 ImmediateFreeLoadedAmmoCount = FMath::Max(
		WeaponAmmoRuntime->LoadedAmmoCount - WeaponAmmoRuntime->ReservedFireTransactionAmmoCount,
		0);

	// [v1.2.0] 현재 장전량만으로 실제 예약 가능한 최대 Launcher 발수입니다.
	const int32 MaximumReservableShotCount = AmmoUnitsPerShot > 0
		? ImmediateFreeLoadedAmmoCount / AmmoUnitsPerShot
		: 0;
	if (MaximumReservableShotCount <= 0
		|| (!bAllowPartialSequence && MaximumReservableShotCount < RequestedShotCount))
	{
		++WeaponAmmoRuntime->NoAmmoRejectCount;
		return ECFAmmoTransactionResult::NotEnoughLoadedAmmo;
	}

	OutReservedShotCount = FMath::Min(RequestedShotCount, MaximumReservableShotCount);
	WeaponAmmoRuntime->ReservedSequenceAmmoCount = OutReservedShotCount * AmmoUnitsPerShot;
	WeaponAmmoRuntime->FiredAmmoCountThisSequence = 0;
	WeaponAmmoRuntime->bWeaponActionLocked = true;
	WeaponAmmoRuntime->WeaponActionLockReason = ECFWeaponActionLockReason::LauncherSequenceActive;
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.2.0] 실제 성공한 Launcher 내부 발사 한 발의 예약량을 Loaded에서 소비로 확정합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::CommitReservedLauncherShot(const FName WeaponInstanceId)
{
	// [v1.2.0] 성공한 예약 발사의 탄약을 실제 소비할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime || !IsValid(WeaponAmmoRuntime->WeaponData))
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	// [v1.2.0] 이번 성공 Launcher 발사 한 발이 소비할 탄약 단위 수입니다.
	const int32 AmmoUnitsPerShot = WeaponAmmoRuntime->WeaponData->GetEffectiveAmmoUnitsPerShot();
	if (WeaponAmmoRuntime->ReservedSequenceAmmoCount < AmmoUnitsPerShot
		|| WeaponAmmoRuntime->LoadedAmmoCount < AmmoUnitsPerShot)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	WeaponAmmoRuntime->LoadedAmmoCount -= AmmoUnitsPerShot;
	WeaponAmmoRuntime->ReservedSequenceAmmoCount -= AmmoUnitsPerShot;
	WeaponAmmoRuntime->FiredAmmoCountThisSequence += AmmoUnitsPerShot;
	WeaponAmmoRuntime->FiredAmmoCountThisSortie += AmmoUnitsPerShot;
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.2.0] 실행 실패한 Launcher 내부 발사 한 발의 예약량을 Loaded 소비 없이 자유 장전량으로 반환합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::ReleaseReservedLauncherShot(const FName WeaponInstanceId)
{
	// [v1.2.0] 실패한 한 발 분량의 예약만 해제할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime || !IsValid(WeaponAmmoRuntime->WeaponData))
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	// [v1.2.0] 실패한 Launcher 내부 발사 한 발에 대응하는 예약 탄약 단위 수입니다.
	const int32 AmmoUnitsPerShot = WeaponAmmoRuntime->WeaponData->GetEffectiveAmmoUnitsPerShot();
	if (WeaponAmmoRuntime->ReservedSequenceAmmoCount < AmmoUnitsPerShot)
	{
		return ECFAmmoTransactionResult::InvalidAmmoAmount;
	}

	WeaponAmmoRuntime->ReservedSequenceAmmoCount -= AmmoUnitsPerShot;
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.2.0] 완료·취소·시작 실패 시 아직 남은 Launcher 예약 전체를 반환하고 Action Lock을 해제합니다.
ECFAmmoTransactionResult UCFVehicleAmmoComp::ReleaseLauncherSequenceReservation(const FName WeaponInstanceId)
{
	// [v1.2.0] 미실행 예약 전체와 Launcher Action Lock을 정리할 WeaponInstance Runtime입니다.
	FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

		WeaponAmmoRuntime->ReservedSequenceAmmoCount = 0;
	if (WeaponAmmoRuntime->WeaponActionLockReason == ECFWeaponActionLockReason::LauncherSequenceActive)
	{
		WeaponAmmoRuntime->bWeaponActionLocked = false;
		WeaponAmmoRuntime->WeaponActionLockReason = ECFWeaponActionLockReason::None;
	}
	UpdateWeaponReloadStateAfterAmmoChange(*WeaponAmmoRuntime);
	if (!TryStartAutoReloadInternal(*WeaponAmmoRuntime))
	{
		OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(*WeaponAmmoRuntime));
	}
	return ECFAmmoTransactionResult::Accepted;
}

// [v1.2.0] 지정 무기가 현재 Launcher Sequence 예약 또는 LauncherSequenceActive Action Lock을 소유하는지 반환합니다.
bool UCFVehicleAmmoComp::HasActiveLauncherSequenceReservation(const FName WeaponInstanceId) const
{
	// [v1.2.0] Launcher 예약·Action Lock 상태를 확인할 WeaponInstance Runtime입니다.
	const FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	return WeaponAmmoRuntime
		&& (WeaponAmmoRuntime->ReservedSequenceAmmoCount > 0
			|| (WeaponAmmoRuntime->bWeaponActionLocked
				&& WeaponAmmoRuntime->WeaponActionLockReason == ECFWeaponActionLockReason::LauncherSequenceActive));
}

// [v1.1.0] 장전·예약·예비량 변화 뒤 현재 무기가 Ready·Empty·NoReserveAmmo 중 어느 상태인지 다시 계산합니다.
void UCFVehicleAmmoComp::UpdateWeaponReloadStateAfterAmmoChange(FCFWeaponAmmoRuntime& WeaponAmmoRuntime)
{
	if (WeaponAmmoRuntime.ReloadState == ECFWeaponReloadState::Reloading
		|| WeaponAmmoRuntime.ReloadState == ECFWeaponReloadState::Disabled)
	{
		return;
	}
	if (!WeaponAmmoRuntime.WeaponData || !WeaponAmmoRuntime.CurrentAmmoData)
	{
		WeaponAmmoRuntime.ReloadState = ECFWeaponReloadState::NotInitialized;
		return;
	}

	// [v1.1.0] 현재 무기가 한 번 발사하기 위해 필요로 하는 탄약 단위 수입니다.
	const int32 RequiredAmmoUnits = WeaponAmmoRuntime.WeaponData->GetEffectiveAmmoUnitsPerShot();

	// [v1.1.0] 현재 예약을 제외하고 새 발사에 사용할 수 있는 자유 장전량입니다.
	const int32 ImmediateFreeLoadedAmmoCount = FMath::Max(
		WeaponAmmoRuntime.LoadedAmmoCount
			- WeaponAmmoRuntime.ReservedSequenceAmmoCount
			- WeaponAmmoRuntime.ReservedFireTransactionAmmoCount,
		0);
	if (ImmediateFreeLoadedAmmoCount >= RequiredAmmoUnits)
	{
		WeaponAmmoRuntime.ReloadState = ECFWeaponReloadState::Ready;
		return;
	}

	// [v1.1.0] 탄창은 발사 불가지만 후속 Reload에 사용할 같은 탄종 차량 예비량입니다.
	const int32 ReserveAmmoCount = GetReserveAmmoCount(WeaponAmmoRuntime.CurrentAmmoData->AmmoId);
	WeaponAmmoRuntime.ReloadState = ReserveAmmoCount > 0
		? ECFWeaponReloadState::Empty
		: ECFWeaponReloadState::NoReserveAmmo;
}

// [v1.0.0] 지정 WeaponInstanceId의 계산 완료 Snapshot을 반환합니다.
bool UCFVehicleAmmoComp::TryGetAmmoSnapshot(const FName WeaponInstanceId, FCFAmmoRuntimeSnapshot& OutAmmoSnapshot) const
{
	OutAmmoSnapshot = FCFAmmoRuntimeSnapshot();
	if (!bAmmoRuntimeInitialized || WeaponInstanceId.IsNone())
	{
		return false;
	}

	const FCFWeaponAmmoRuntime* WeaponAmmoRuntime = WeaponAmmoRuntimeByInstanceId.Find(WeaponInstanceId);
	if (!WeaponAmmoRuntime)
	{
		return false;
	}

	OutAmmoSnapshot = BuildAmmoSnapshot(*WeaponAmmoRuntime);
	return true;
}

// [v1.0.0] 현재 VehicleWeaponComp 활성 MountProfileId의 탄약 Snapshot을 반환합니다.
bool UCFVehicleAmmoComp::TryGetActiveWeaponAmmoSnapshot(
	const UCFVehicleWeaponComp* VehicleWeaponComp,
	FCFAmmoRuntimeSnapshot& OutAmmoSnapshot) const
{
	OutAmmoSnapshot = FCFAmmoRuntimeSnapshot();
	return VehicleWeaponComp
		&& TryGetAmmoSnapshot(VehicleWeaponComp->GetActiveMountProfileId(), OutAmmoSnapshot);
}

// [v1.0.0] 지정 탄종 ID의 차량 예비 탄약 수를 반환합니다.
int32 UCFVehicleAmmoComp::GetReserveAmmoCount(const FName AmmoId) const
{
	return FMath::Max(ReserveAmmoCountByAmmoId.FindRef(AmmoId), 0);
}

// [v1.0.0] 내부 Weapon Runtime 한 건에서 UI·Debug용 계산 완료 Snapshot을 생성합니다.
FCFAmmoRuntimeSnapshot UCFVehicleAmmoComp::BuildAmmoSnapshot(const FCFWeaponAmmoRuntime& WeaponAmmoRuntime) const
{
	FCFAmmoRuntimeSnapshot AmmoSnapshot;
	AmmoSnapshot.WeaponInstanceId = WeaponAmmoRuntime.WeaponInstanceId;
	AmmoSnapshot.AmmoId = WeaponAmmoRuntime.CurrentAmmoData
		? WeaponAmmoRuntime.CurrentAmmoData->AmmoId
		: NAME_None;
	AmmoSnapshot.bFiniteAmmoRuntimeActive = !AmmoSnapshot.AmmoId.IsNone();
	AmmoSnapshot.MagazineCapacity = FMath::Max(WeaponAmmoRuntime.MagazineCapacity, 0);
	AmmoSnapshot.LoadedAmmoCount = FMath::Max(WeaponAmmoRuntime.LoadedAmmoCount, 0);
		AmmoSnapshot.ReservedSequenceAmmoCount = FMath::Clamp(
		WeaponAmmoRuntime.ReservedSequenceAmmoCount,
		0,
		AmmoSnapshot.LoadedAmmoCount);

	// [v1.1.0] SingleCycle 실행 중 임시 예약되어 신규 행동에서 자유롭게 사용할 수 없는 탄약량입니다.
	const int32 ReservedFireTransactionAmmoCount = FMath::Clamp(
		WeaponAmmoRuntime.ReservedFireTransactionAmmoCount,
		0,
		FMath::Max(AmmoSnapshot.LoadedAmmoCount - AmmoSnapshot.ReservedSequenceAmmoCount, 0));
	AmmoSnapshot.ImmediateUsableAmmoCount = FMath::Max(
		AmmoSnapshot.LoadedAmmoCount
			- AmmoSnapshot.ReservedSequenceAmmoCount
			- ReservedFireTransactionAmmoCount,
		0);
	AmmoSnapshot.ReserveAmmoCount = GetReserveAmmoCount(AmmoSnapshot.AmmoId);
	AmmoSnapshot.CurrentUsableAmmoCount = AmmoSnapshot.ImmediateUsableAmmoCount + AmmoSnapshot.ReserveAmmoCount;
	AmmoSnapshot.CurrentOnboardAmmoCount = CalculateCurrentOnboardAmmoCount(AmmoSnapshot.AmmoId);

	// [v1.0.0] 한 번 발사에 필요한 실제 탄약 단위 수입니다.
	const int32 AmmoUnitsPerShot = WeaponAmmoRuntime.WeaponData
		? WeaponAmmoRuntime.WeaponData->GetEffectiveAmmoUnitsPerShot()
		: 1;
	AmmoSnapshot.ImmediateFireableShotCount = AmmoUnitsPerShot > 0
		? AmmoSnapshot.ImmediateUsableAmmoCount / AmmoUnitsPerShot
		: 0;
	AmmoSnapshot.RemainingFreeFireableShotCount = AmmoUnitsPerShot > 0
		? AmmoSnapshot.CurrentUsableAmmoCount / AmmoUnitsPerShot
		: 0;
	AmmoSnapshot.PendingSequenceShotCount = AmmoUnitsPerShot > 0
		? AmmoSnapshot.ReservedSequenceAmmoCount / AmmoUnitsPerShot
		: 0;
	AmmoSnapshot.FiredAmmoCountThisSequence = FMath::Max(WeaponAmmoRuntime.FiredAmmoCountThisSequence, 0);
		AmmoSnapshot.FiredAmmoCountThisSortie = FMath::Max(WeaponAmmoRuntime.FiredAmmoCountThisSortie, 0);
	AmmoSnapshot.ReloadState = WeaponAmmoRuntime.ReloadState;
	AmmoSnapshot.ReloadDurationSeconds = FMath::Max(WeaponAmmoRuntime.ReloadDurationSeconds, 0.0f);
	AmmoSnapshot.RemainingReloadTimeSeconds = FMath::Max(
		AmmoSnapshot.ReloadDurationSeconds - WeaponAmmoRuntime.ReloadElapsedSeconds,
		0.0f);
	AmmoSnapshot.bWeaponActionLocked = WeaponAmmoRuntime.bWeaponActionLocked;
	AmmoSnapshot.WeaponActionLockReason = WeaponAmmoRuntime.WeaponActionLockReason;
	return AmmoSnapshot;
}

// [v1.0.0] 같은 AmmoId를 사용하는 모든 무기 장전량과 차량 예비량을 합친 실제 보유량을 계산합니다.
int32 UCFVehicleAmmoComp::CalculateCurrentOnboardAmmoCount(const FName AmmoId) const
{
	if (AmmoId.IsNone())
	{
		return 0;
	}

	int32 CurrentOnboardAmmoCount = GetReserveAmmoCount(AmmoId);
	for (const TPair<FName, FCFWeaponAmmoRuntime>& WeaponAmmoPair : WeaponAmmoRuntimeByInstanceId)
	{
		const FCFWeaponAmmoRuntime& WeaponAmmoRuntime = WeaponAmmoPair.Value;
		if (WeaponAmmoRuntime.CurrentAmmoData && WeaponAmmoRuntime.CurrentAmmoData->AmmoId == AmmoId)
		{
			CurrentOnboardAmmoCount += FMath::Max(WeaponAmmoRuntime.LoadedAmmoCount, 0);
		}
	}
	return CurrentOnboardAmmoCount;
}

// [v1.0.0] 현재 모든 유한탄 WeaponInstance Snapshot을 상태 변경 이벤트로 전달합니다.
void UCFVehicleAmmoComp::BroadcastAllAmmoSnapshots()
{
	for (const TPair<FName, FCFWeaponAmmoRuntime>& WeaponAmmoPair : WeaponAmmoRuntimeByInstanceId)
	{
		OnAmmoRuntimeChanged.Broadcast(BuildAmmoSnapshot(WeaponAmmoPair.Value));
	}
}
