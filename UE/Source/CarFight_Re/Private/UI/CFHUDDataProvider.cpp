// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-08-13
// Description: CF-FQ-032 UI-P0-03 + CF-FQ-031 AMMO-P0-06 HUD Data Provider 구현
// Scope: Current Pawn Rebind, Gameplay·Ammo 이벤트와 10Hz Game-Time 갱신을 통합 ViewData로 변환합니다.
// Changelog:
// - v1.5.0: Launcher 상태 이벤트 전용 LauncherSequenceRevision을 ViewData에 전달해 공통 Presentation lifecycle이 Ammo/Timer Refresh와 실제 Sequence 전이를 구분하도록 교정.
// - v1.4.0: finite Ammo Snapshot의 MagazineCapacity를 HUD ViewData로 전달해 Loaded/Capacity 표시 계약을 지원.
// - v1.3.0: VehicleAmmoComp 이벤트를 Pawn Rebind 수명에 연결하고 실제 finite Ammo·Reload Snapshot을 Weapon ViewData로 변환.
// - v1.2.0: 정상 Ripple/Salvo 진행은 Weapon ViewData만 소유하도록 정리하고 AlertFeed용 LauncherSequence 상태 Alert 생성을 제거.
// - v1.1.0: Launcher Sequence 상태 이벤트를 Pawn Rebind 수명에 구독해 Weapon/Alert ViewData를 즉시 갱신.
// - v1.0.0: Vehicle/Weapon/Defense/Target/Alert 실제 Runtime 변환과 Radar/Ammo/Heat/RPM/Gear Unavailable 계약을 구현.
// Migration:
// - Gameplay 계산을 변경하지 않으며 모든 값은 기존 public Getter/Event의 읽기 전용 소비입니다.
// - finite Ammo Runtime Snapshot이 없으면 AmmoAvailability를 Unavailable로 유지하며 MagazineSize나 MaximumLoadableAmmoCount를 현재 탄약으로 사용하지 않습니다.
// - Widget 또는 Presenter에서 Gameplay Actor/Component를 찾지 않습니다.

#include "UI/CFHUDDataProvider.h"

#include "CFLauncherComp.h"
#include "CFTargetSelectComp.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "Engine/World.h"
#include "UI/CFUISubsystem.h"

// [v1.0.0] UISubsystem의 Current Pawn 변경 이벤트를 구독하고 현재 Pawn을 즉시 Rebind합니다.
bool UCFHUDDataProvider::InitializeProvider(UCFUISubsystem* InUISubsystem)
{
	if (!InUISubsystem)
	{
		return false;
	}

	if (UISubsystem == InUISubsystem)
	{
		RebindCurrentPawn(UISubsystem->GetCurrentPawn());
		return true;
	}

	ShutdownProvider();
	UISubsystem = InUISubsystem;
	UISubsystem->OnCurrentPawnChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleCurrentPawnChanged);
	UISubsystem->OnCurrentPawnChanged.AddDynamic(this, &UCFHUDDataProvider::HandleCurrentPawnChanged);
	RebindCurrentPawn(UISubsystem->GetCurrentPawn());
	return true;
}

// [v1.0.0] UISubsystem과 Gameplay Component 이벤트, 연속값 Timer를 모두 해제합니다.
void UCFHUDDataProvider::ShutdownProvider()
{
	StopContinuousRefreshTimer();
	UnbindGameplayEvents();

	if (UISubsystem)
	{
		UISubsystem->OnCurrentPawnChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleCurrentPawnChanged);
	}

	UISubsystem = nullptr;
	BoundVehiclePawn.Reset();
	BoundWorld.Reset();
	++BindingGeneration;
	RefreshViewData();
}

// [v1.0.0] 지정 Pawn을 새 UI Gameplay Source로 바꾸고 이전 Pawn 구독을 먼저 제거합니다.
void UCFHUDDataProvider::RebindCurrentPawn(APawn* NewPawn)
{
	// [v1.0.0] Provider 계층에서만 허용되는 실제 차량 Pawn Source Cast 결과입니다.
	ACFVehiclePawn* NewVehiclePawn = Cast<ACFVehiclePawn>(NewPawn);
	if (BoundVehiclePawn.Get() == NewVehiclePawn)
	{
		RefreshViewData();
		return;
	}

	StopContinuousRefreshTimer();
	UnbindGameplayEvents();
		BoundVehiclePawn = NewVehiclePawn;
	BoundWorld = NewVehiclePawn ? NewVehiclePawn->GetWorld() : nullptr;
	// [v1.5.0] Launcher 의미 Revision은 Pawn Binding 수명 안에서만 비교합니다.
	LauncherSequenceRevision = 0;
	++BindingGeneration;
	BindGameplayEvents();
	StartContinuousRefreshTimer();
	RefreshViewData();
}

// [v1.0.0] 현재 Bound Pawn의 실제 Runtime에서 통합 ViewData를 다시 계산하고 Broadcast합니다.
void UCFHUDDataProvider::RefreshViewData()
{
	// [v1.0.0] 이번 갱신에서 새로 만드는 전체 HUD ViewData입니다.
	FCFInGameUIViewData NewViewData;
	NewViewData.Revision = ++ViewDataRevision;
	NewViewData.BindingGeneration = BindingGeneration;
	FillVehicleViewData(NewViewData.Vehicle);
	FillDefenseViewData(NewViewData.Defense);
	FillWeaponViewData(NewViewData.Weapon);
	FillTargetViewData(NewViewData.Target);
	FillRadarViewData(NewViewData.Radar);
	FillAlertViewData(NewViewData.Alerts);
	CurrentViewData = NewViewData;
	OnHUDViewDataChanged.Broadcast(CurrentViewData);
}

// [v1.0.0] UISubsystem OnCurrentPawnChanged를 Provider Rebind로 변환합니다.
void UCFHUDDataProvider::HandleCurrentPawnChanged(APawn* PreviousPawn, APawn* NewPawn)
{
	(void)PreviousPawn;
	RebindCurrentPawn(NewPawn);
}

// [v1.0.0] VehicleHealth 값 변경을 즉시 ViewData Refresh로 반영합니다.
void UCFHUDDataProvider::HandleVehicleHealthChanged(float PreviousHealth, float CurrentHealth, float MaxHealth)
{
	(void)PreviousHealth;
	(void)CurrentHealth;
	(void)MaxHealth;
	RefreshViewData();
}

// [v1.0.0] Vehicle Destroyed 전이를 즉시 ViewData와 Alert에 반영합니다.
void UCFHUDDataProvider::HandleVehicleDestroyed(FCFDamageHitContext DamageHitContext)
{
	(void)DamageHitContext;
	RefreshViewData();
}

// [v1.0.0] Shield 변경을 즉시 ViewData Refresh로 반영합니다.
void UCFHUDDataProvider::HandleShieldChanged(float PreviousShield, float CurrentShield, float MaximumShield)
{
	(void)PreviousShield;
	(void)CurrentShield;
	(void)MaximumShield;
	RefreshViewData();
}

// [v1.0.0] 방향 Armor 변경을 즉시 ViewData Refresh로 반영합니다.
void UCFHUDDataProvider::HandleArmorChanged(ECFArmorDirection ArmorDirection, float PreviousArmor, float CurrentArmor, float MaximumArmor)
{
	(void)ArmorDirection;
	(void)PreviousArmor;
	(void)CurrentArmor;
	(void)MaximumArmor;
	RefreshViewData();
}

// [v1.5.0] Launcher Sequence 시작·진행·완료·취소 상태를 의미 Revision과 함께 즉시 Weapon ViewData로 반영합니다.
void UCFHUDDataProvider::HandleLauncherSequenceChanged(FCFLauncherSequenceRuntime SequenceRuntime)
{
	(void)SequenceRuntime;
	// [v1.5.0] Ammo 변경·10Hz Timer Refresh와 구분되는 실제 Launcher 상태 이벤트 번호입니다.
	++LauncherSequenceRevision;
	RefreshViewData();
}

// [v1.3.0] 장전·예비·예약·Reload·Action Lock 변경을 즉시 Weapon ViewData로 반영합니다.
void UCFHUDDataProvider::HandleAmmoRuntimeChanged(FCFAmmoRuntimeSnapshot AmmoSnapshot)
{
	(void)AmmoSnapshot;
	RefreshViewData();
}

// [v1.0.0] 선택 Target 교체를 즉시 Player-facing Target ViewData로 반영합니다.
void UCFHUDDataProvider::HandleSelectedTargetChanged(AActor* PreviousTarget, AActor* NewTarget, FCFTargetDisplayInfo DisplayInfo)
{
	(void)PreviousTarget;
	(void)NewTarget;
	(void)DisplayInfo;
	RefreshViewData();
}

// [v1.0.0] 선택 Target 해제를 즉시 ViewData에 반영합니다.
void UCFHUDDataProvider::HandleSelectedTargetCleared(AActor* ClearedTarget, ECFTargetClearReason ClearReason)
{
	(void)ClearedTarget;
	(void)ClearReason;
	RefreshViewData();
}

// [v1.0.0] 선택 Target 유효성 전이를 즉시 ViewData에 반영합니다.
void UCFHUDDataProvider::HandleSelectedTargetValidityChanged(AActor* TargetActor, bool bIsValidTarget)
{
	(void)TargetActor;
	(void)bIsValidTarget;
	RefreshViewData();
}

// [v1.0.0] 선택 Target 추적 상태 전이를 즉시 ViewData에 반영합니다.
void UCFHUDDataProvider::HandleSelectedTargetTrackStateChanged(AActor* TargetActor, ECFTargetTrackState PreviousState, ECFTargetTrackState NewState)
{
	(void)TargetActor;
	(void)PreviousState;
	(void)NewState;
	RefreshViewData();
}

// [v1.1.0] 이전 Pawn의 Health/Defense/Launcher/Target 이벤트 구독을 모두 제거합니다.
void UCFHUDDataProvider::UnbindGameplayEvents()
{
	if (UCFVehicleHealthComp* HealthComponent = BoundHealthComponent.Get())
	{
		HealthComponent->OnVehicleHealthChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleVehicleHealthChanged);
		HealthComponent->OnVehicleDestroyed.RemoveDynamic(this, &UCFHUDDataProvider::HandleVehicleDestroyed);
	}

	if (UCFVehicleDefenseComp* DefenseComponent = BoundDefenseComponent.Get())
	{
		DefenseComponent->OnShieldChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleShieldChanged);
		DefenseComponent->OnArmorChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleArmorChanged);
	}

		if (UCFLauncherComp* LauncherComponent = BoundLauncherComponent.Get())
	{
		LauncherComponent->OnLauncherSequenceChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleLauncherSequenceChanged);
	}

	if (UCFVehicleAmmoComp* AmmoComponent = BoundAmmoComponent.Get())
	{
		AmmoComponent->OnAmmoRuntimeChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleAmmoRuntimeChanged);
	}

	if (UCFTargetSelectComp* TargetSelectComponent = BoundTargetSelectComponent.Get())
	{
		TargetSelectComponent->OnSelectedTargetChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetChanged);
		TargetSelectComponent->OnSelectedTargetCleared.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetCleared);
		TargetSelectComponent->OnSelectedTargetValidityChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetValidityChanged);
		TargetSelectComponent->OnSelectedTargetTrackStateChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetTrackStateChanged);
	}

		BoundHealthComponent.Reset();
	BoundDefenseComponent.Reset();
	BoundLauncherComponent.Reset();
	BoundAmmoComponent.Reset();
	BoundTargetSelectComponent.Reset();
}

// [v1.1.0] 새 Pawn의 Health/Defense/Launcher/Target 이벤트를 Provider에 연결합니다.
void UCFHUDDataProvider::BindGameplayEvents()
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

		BoundHealthComponent = VehiclePawn->GetVehicleHealthComp();
	BoundDefenseComponent = VehiclePawn->GetVehicleDefenseComp();
	BoundLauncherComponent = VehiclePawn->GetLauncherComp();
	BoundAmmoComponent = VehiclePawn->GetVehicleAmmoComp();
	BoundTargetSelectComponent = VehiclePawn->GetTargetSelectComp();

	if (UCFVehicleHealthComp* HealthComponent = BoundHealthComponent.Get())
	{
		HealthComponent->OnVehicleHealthChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleVehicleHealthChanged);
		HealthComponent->OnVehicleHealthChanged.AddDynamic(this, &UCFHUDDataProvider::HandleVehicleHealthChanged);
		HealthComponent->OnVehicleDestroyed.RemoveDynamic(this, &UCFHUDDataProvider::HandleVehicleDestroyed);
		HealthComponent->OnVehicleDestroyed.AddDynamic(this, &UCFHUDDataProvider::HandleVehicleDestroyed);
	}

	if (UCFVehicleDefenseComp* DefenseComponent = BoundDefenseComponent.Get())
	{
		DefenseComponent->OnShieldChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleShieldChanged);
		DefenseComponent->OnShieldChanged.AddDynamic(this, &UCFHUDDataProvider::HandleShieldChanged);
		DefenseComponent->OnArmorChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleArmorChanged);
		DefenseComponent->OnArmorChanged.AddDynamic(this, &UCFHUDDataProvider::HandleArmorChanged);
	}

		if (UCFLauncherComp* LauncherComponent = BoundLauncherComponent.Get())
	{
		LauncherComponent->OnLauncherSequenceChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleLauncherSequenceChanged);
		LauncherComponent->OnLauncherSequenceChanged.AddDynamic(this, &UCFHUDDataProvider::HandleLauncherSequenceChanged);
	}

	if (UCFVehicleAmmoComp* AmmoComponent = BoundAmmoComponent.Get())
	{
		AmmoComponent->OnAmmoRuntimeChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleAmmoRuntimeChanged);
		AmmoComponent->OnAmmoRuntimeChanged.AddDynamic(this, &UCFHUDDataProvider::HandleAmmoRuntimeChanged);
	}

	if (UCFTargetSelectComp* TargetSelectComponent = BoundTargetSelectComponent.Get())
	{
		TargetSelectComponent->OnSelectedTargetChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetChanged);
		TargetSelectComponent->OnSelectedTargetChanged.AddDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetChanged);
		TargetSelectComponent->OnSelectedTargetCleared.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetCleared);
		TargetSelectComponent->OnSelectedTargetCleared.AddDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetCleared);
		TargetSelectComponent->OnSelectedTargetValidityChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetValidityChanged);
		TargetSelectComponent->OnSelectedTargetValidityChanged.AddDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetValidityChanged);
		TargetSelectComponent->OnSelectedTargetTrackStateChanged.RemoveDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetTrackStateChanged);
		TargetSelectComponent->OnSelectedTargetTrackStateChanged.AddDynamic(this, &UCFHUDDataProvider::HandleSelectedTargetTrackStateChanged);
	}
}

// [v1.0.0] 속도·쿨다운·Launcher 진행처럼 연속 변화하는 값의 10Hz Game-Time 갱신 Timer를 시작합니다.
void UCFHUDDataProvider::StartContinuousRefreshTimer()
{
	UWorld* World = BoundWorld.Get();
	if (!World || ContinuousRefreshIntervalSeconds <= 0.0f)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		ContinuousRefreshTimerHandle,
		this,
		&UCFHUDDataProvider::RefreshViewData,
		ContinuousRefreshIntervalSeconds,
		true,
		ContinuousRefreshIntervalSeconds);
}

// [v1.0.0] 이전 Pawn World의 연속값 갱신 Timer를 제거합니다.
void UCFHUDDataProvider::StopContinuousRefreshTimer()
{
	if (UWorld* World = BoundWorld.Get())
	{
		World->GetTimerManager().ClearTimer(ContinuousRefreshTimerHandle);
	}
	ContinuousRefreshTimerHandle.Invalidate();
}

// [v1.0.0] 현재 차량의 주행 ViewData를 채웁니다.
void UCFHUDDataProvider::FillVehicleViewData(FCFVehicleHUDData& OutVehicleViewData) const
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	OutVehicleViewData.Availability = ECFUIViewAvailability::Known;
	OutVehicleViewData.SpeedKmh = FMath::Max(0.0f, FMath::Abs(VehiclePawn->GetVehicleSpeed()));
	OutVehicleViewData.SpeedAvailability = ResolveKnownNumericAvailability(true, OutVehicleViewData.SpeedKmh);
	OutVehicleViewData.EngineRpmAvailability = ECFUIViewAvailability::Unavailable;
	OutVehicleViewData.GearAvailability = ECFUIViewAvailability::Unavailable;
	OutVehicleViewData.bVehicleCoreRuntimeReady = VehiclePawn->bVehicleCoreRuntimeReady;
	OutVehicleViewData.bVehicleCombatRuntimeReady = VehiclePawn->bVehicleCombatRuntimeReady;
}

// [v1.0.0] 현재 차량의 Defense/Integrity ViewData를 채웁니다.
void UCFHUDDataProvider::FillDefenseViewData(FCFDefenseHUDData& OutDefenseViewData) const
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	UCFVehicleHealthComp* HealthComponent = VehiclePawn->GetVehicleHealthComp();
	if (HealthComponent && HealthComponent->IsHealthInitialized())
	{
		OutDefenseViewData.CurrentIntegrity = HealthComponent->GetCurrentIntegrity();
		OutDefenseViewData.MaximumIntegrity = HealthComponent->GetMaximumIntegrity();
		OutDefenseViewData.IntegrityRatio = HealthComponent->GetIntegrityRatio();
		OutDefenseViewData.IntegrityAvailability = ResolveKnownNumericAvailability(true, OutDefenseViewData.CurrentIntegrity);
		OutDefenseViewData.bDestroyed = HealthComponent->IsDestroyed();
	}

	UCFVehicleDefenseComp* DefenseComponent = VehiclePawn->GetVehicleDefenseComp();
	if (!DefenseComponent || !DefenseComponent->IsDefenseInitialized())
	{
		OutDefenseViewData.Availability = ECFUIViewAvailability::Unavailable;
		OutDefenseViewData.bUsingLegacyFallback = HealthComponent != nullptr;
		return;
	}

	OutDefenseViewData.Availability = ECFUIViewAvailability::Known;
	OutDefenseViewData.bUsingLegacyFallback = false;
	OutDefenseViewData.CurrentShield = DefenseComponent->GetCurrentShield();
	OutDefenseViewData.MaximumShield = DefenseComponent->GetMaximumShield();
	OutDefenseViewData.ShieldRatio = DefenseComponent->GetShieldRatio();
	OutDefenseViewData.ShieldAvailability = ResolveKnownNumericAvailability(true, OutDefenseViewData.CurrentShield);
	OutDefenseViewData.bShieldRegenerating = DefenseComponent->IsShieldRegenerating();
	OutDefenseViewData.ArmorAvailability = ECFUIViewAvailability::Known;
	OutDefenseViewData.FrontArmorRatio = DefenseComponent->GetArmorRatio(ECFArmorDirection::Front);
	OutDefenseViewData.RightArmorRatio = DefenseComponent->GetArmorRatio(ECFArmorDirection::Right);
	OutDefenseViewData.RearArmorRatio = DefenseComponent->GetArmorRatio(ECFArmorDirection::Rear);
	OutDefenseViewData.LeftArmorRatio = DefenseComponent->GetArmorRatio(ECFArmorDirection::Left);
	OutDefenseViewData.TopArmorRatio = DefenseComponent->GetArmorRatio(ECFArmorDirection::Top);
	OutDefenseViewData.BottomArmorRatio = DefenseComponent->GetArmorRatio(ECFArmorDirection::Bottom);
}

// [v1.0.0] 현재 활성 Weapon과 Launcher ViewData를 채웁니다.
void UCFHUDDataProvider::FillWeaponViewData(FCFWeaponHUDData& OutWeaponViewData) const
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	// [v1.0.0] 기존 Weapon Runtime의 public Debug Snapshot을 읽기 전용 UI Adapter 입력으로 사용합니다.
	const FCFVehicleDebugWeapon WeaponDebug = VehiclePawn->GetVehicleDebugWeapon();
	if (!WeaponDebug.bHasVehicleWeaponComponent)
	{
		return;
	}

	if (!WeaponDebug.ActiveWeaponData)
	{
		OutWeaponViewData.Availability = ECFUIViewAvailability::KnownZero;
	}
	else
	{
		OutWeaponViewData.Availability = ECFUIViewAvailability::Known;
		OutWeaponViewData.WeaponId = WeaponDebug.ActiveWeaponId;
		OutWeaponViewData.DisplayNameAvailability = ECFUIViewAvailability::Unavailable;
		OutWeaponViewData.CooldownDurationSeconds = FMath::Max(0.0f, WeaponDebug.ActiveWeaponCooldownSeconds);
		OutWeaponViewData.RemainingCooldownSeconds = FMath::Max(0.0f, WeaponDebug.ActiveWeaponRemainingCooldownSeconds);
		OutWeaponViewData.CooldownAvailability = ResolveKnownNumericAvailability(
			OutWeaponViewData.CooldownDurationSeconds > 0.0f,
			OutWeaponViewData.RemainingCooldownSeconds);
	}

		OutWeaponViewData.AmmoAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.3.0] 현재 활성 WeaponInstance의 실제 finite Ammo Runtime Snapshot입니다.
	FCFAmmoRuntimeSnapshot AmmoSnapshot;
	UCFVehicleAmmoComp* AmmoComponent = VehiclePawn->GetVehicleAmmoComp();
	if (AmmoComponent
		&& AmmoComponent->TryGetActiveWeaponAmmoSnapshot(VehiclePawn->GetVehicleWeaponComp(), AmmoSnapshot)
		&& AmmoSnapshot.bFiniteAmmoRuntimeActive)
	{
		OutWeaponViewData.AmmoAvailability = AmmoSnapshot.CurrentOnboardAmmoCount > 0
			? ECFUIViewAvailability::Known
			: ECFUIViewAvailability::KnownZero;
				OutWeaponViewData.LoadedAmmoCount = FMath::Max(AmmoSnapshot.LoadedAmmoCount, 0);
		OutWeaponViewData.MagazineCapacity = FMath::Max(AmmoSnapshot.MagazineCapacity, 0);
		OutWeaponViewData.ReservedSequenceAmmoCount = FMath::Max(AmmoSnapshot.ReservedSequenceAmmoCount, 0);
		OutWeaponViewData.ImmediateUsableAmmoCount = FMath::Max(AmmoSnapshot.ImmediateUsableAmmoCount, 0);
		OutWeaponViewData.ReserveAmmoCount = FMath::Max(AmmoSnapshot.ReserveAmmoCount, 0);
		OutWeaponViewData.CurrentUsableAmmoCount = FMath::Max(AmmoSnapshot.CurrentUsableAmmoCount, 0);
		OutWeaponViewData.CurrentOnboardAmmoCount = FMath::Max(AmmoSnapshot.CurrentOnboardAmmoCount, 0);
		OutWeaponViewData.ReloadState = AmmoSnapshot.ReloadState;
		OutWeaponViewData.ReloadDurationSeconds = FMath::Max(AmmoSnapshot.ReloadDurationSeconds, 0.0f);
		OutWeaponViewData.RemainingReloadTimeSeconds = FMath::Max(AmmoSnapshot.RemainingReloadTimeSeconds, 0.0f);
		OutWeaponViewData.bWeaponActionLocked = AmmoSnapshot.bWeaponActionLocked;
		OutWeaponViewData.WeaponActionLockReason = AmmoSnapshot.WeaponActionLockReason;
	}

	OutWeaponViewData.HeatAvailability = ECFUIViewAvailability::Unavailable;

	UCFLauncherComp* LauncherComponent = VehiclePawn->GetLauncherComp();
	if (!LauncherComponent)
	{
		return;
	}

		// [v1.5.0] Presenter가 부수 Refresh와 실제 Launcher 상태 이벤트를 구분할 의미 Revision입니다.
	OutWeaponViewData.LauncherSequenceRevision = LauncherSequenceRevision;

	// [v1.0.0] 현재 또는 마지막 Launcher 시퀀스의 실제 Runtime Snapshot입니다.
	const FCFLauncherSequenceRuntime LauncherRuntime = LauncherComponent->GetLauncherSequenceRuntime();
	OutWeaponViewData.bLauncherSequenceActive = LauncherRuntime.IsActive();
	OutWeaponViewData.LauncherAvailability = OutWeaponViewData.bLauncherSequenceActive
		? ECFUIViewAvailability::Known
		: ECFUIViewAvailability::KnownZero;
	OutWeaponViewData.LauncherPattern = LauncherRuntime.ActiveConfig.FirePattern;
	OutWeaponViewData.LauncherTotalProjectileCount = LauncherRuntime.TotalProjectileCount;
	OutWeaponViewData.LauncherAcceptedProjectileCount = LauncherRuntime.AcceptedProjectileCount;
	OutWeaponViewData.LauncherRemainingProjectileCount = LauncherRuntime.RemainingProjectileCount;
}

// [v1.0.0] TargetSelect 공개 정보를 Target Knowledge ViewData로 좁혀 채웁니다.
void UCFHUDDataProvider::FillTargetViewData(FCFTargetHUDData& OutTargetViewData) const
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	UCFTargetSelectComp* TargetSelectComponent = VehiclePawn->GetTargetSelectComp();
	if (!TargetSelectComponent)
	{
		return;
	}

	if (!TargetSelectComponent->HasSelectedTarget())
	{
		OutTargetViewData.Availability = ECFUIViewAvailability::KnownZero;
		return;
	}

	// [v1.0.0] TargetSelectable이 선택 시점에 명시적으로 공개한 Player-facing DisplayInfo입니다.
	const FCFTargetDisplayInfo DisplayInfo = TargetSelectComponent->GetSelectedTargetDisplayInfo();
	OutTargetViewData.Availability = ECFUIViewAvailability::Known;
	OutTargetViewData.bHasSelectedTarget = true;
	OutTargetViewData.bSelectedTargetValid = TargetSelectComponent->IsSelectedTargetValid();
	OutTargetViewData.TargetId = DisplayInfo.TargetId;
	OutTargetViewData.Relation = DisplayInfo.Relation;
	OutTargetViewData.Category = DisplayInfo.TargetCategory;
	OutTargetViewData.InformationLevel = DisplayInfo.InformationLevel;
	OutTargetViewData.TrackState = TargetSelectComponent->GetSelectedTargetTrackState();

	// [v1.0.0] Detected 단계에서 내부 Actor 이름을 노출하지 않고 Identified 이상에서 공개 DisplayName만 사용합니다.
	const bool bIdentityKnown = DisplayInfo.InformationLevel == ECFTargetInfoLevel::Identified
		|| DisplayInfo.InformationLevel == ECFTargetInfoLevel::DetailedScan;
	if (bIdentityKnown && !DisplayInfo.DisplayName.IsEmpty())
	{
		OutTargetViewData.IdentityAvailability = ECFUIViewAvailability::Known;
		OutTargetViewData.DisplayName = DisplayInfo.DisplayName;
	}
	else
	{
		OutTargetViewData.IdentityAvailability = ECFUIViewAvailability::Unknown;
	}

	OutTargetViewData.DistanceAvailability = ECFUIViewAvailability::Unavailable;
}

// [v1.0.0] 실제 Sensor Provider 부재를 Radar Unavailable로 명시합니다.
void UCFHUDDataProvider::FillRadarViewData(FCFRadarHUDData& OutRadarViewData) const
{
	OutRadarViewData.Availability = ECFUIViewAvailability::Unavailable;
	OutRadarViewData.Contacts.Reset();
}

// [v1.2.0] 실제 Destroyed, Shield Down과 Armor Breach처럼 전역 주의가 필요한 상태만 Alert 목록으로 만듭니다.
void UCFHUDDataProvider::FillAlertViewData(FCFCombatAlertViewData& OutAlertViewData) const
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	OutAlertViewData.Availability = ECFUIViewAvailability::Known;

	UCFVehicleHealthComp* HealthComponent = VehiclePawn->GetVehicleHealthComp();
	if (HealthComponent && HealthComponent->IsDestroyed())
	{
		AddAlertUnique(
			OutAlertViewData.ActiveAlerts,
			FName(TEXT("VehicleDestroyed")),
			ECFHUDAlertPriority::Critical,
			FText::FromString(TEXT("VEHICLE DESTROYED")),
			FText::GetEmpty());
		return;
	}

	UCFVehicleDefenseComp* DefenseComponent = VehiclePawn->GetVehicleDefenseComp();
	if (DefenseComponent && DefenseComponent->IsDefenseInitialized())
	{
		if (DefenseComponent->GetMaximumShield() > KINDA_SMALL_NUMBER
			&& DefenseComponent->GetCurrentShield() <= KINDA_SMALL_NUMBER)
		{
			AddAlertUnique(
				OutAlertViewData.ActiveAlerts,
				FName(TEXT("ShieldDown")),
				ECFHUDAlertPriority::Caution,
				FText::FromString(TEXT("SHIELD DOWN")),
				FText::GetEmpty());
		}

		// [v1.0.0] 실제 최대 Armor가 존재하면서 현재값이 0인 방향 수입니다.
		int32 BrokenArmorDirectionCount = 0;
		// [v1.0.0] 현재 방어 Runtime에서 검사할 정확한 6방향 목록입니다.
		const ECFArmorDirection ArmorDirections[] =
		{
			ECFArmorDirection::Front,
			ECFArmorDirection::Right,
			ECFArmorDirection::Rear,
			ECFArmorDirection::Left,
			ECFArmorDirection::Top,
			ECFArmorDirection::Bottom
		};
		for (const ECFArmorDirection ArmorDirection : ArmorDirections)
		{
			if (DefenseComponent->GetMaximumArmor(ArmorDirection) > KINDA_SMALL_NUMBER
				&& DefenseComponent->GetCurrentArmor(ArmorDirection) <= KINDA_SMALL_NUMBER)
			{
				++BrokenArmorDirectionCount;
			}
		}

		if (BrokenArmorDirectionCount > 0)
		{
			AddAlertUnique(
				OutAlertViewData.ActiveAlerts,
				FName(TEXT("ArmorBreach")),
				ECFHUDAlertPriority::Caution,
				FText::FromString(TEXT("ARMOR BREACH")),
				FText::FromString(FString::Printf(TEXT("%d / 6"), BrokenArmorDirectionCount)));
		}
	}

	
	// [v1.2.0] 정상 Ripple/Salvo 진행은 전역 Alert가 아니라 FCFWeaponHUDData의 Launcher Sequence 채널이 소유합니다.

}

// [v1.0.0] 현재값이 실제 제공됐을 때 0과 비0을 KnownZero/Known으로 구분합니다.
ECFUIViewAvailability UCFHUDDataProvider::ResolveKnownNumericAvailability(const bool bAvailable, const float CurrentValue)
{
	if (!bAvailable)
	{
		return ECFUIViewAvailability::Unavailable;
	}
	return FMath::IsNearlyZero(CurrentValue)
		? ECFUIViewAvailability::KnownZero
		: ECFUIViewAvailability::Known;
}

// [v1.0.0] 현재 Alert 목록에 안정 Key 하나를 중복 없이 추가합니다.
void UCFHUDDataProvider::AddAlertUnique(
	TArray<FCFHUDAlertItem>& Alerts,
	const FName AlertKey,
	const ECFHUDAlertPriority Priority,
	const FText& Title,
	const FText& Detail)
{
	if (AlertKey.IsNone())
	{
		return;
	}

	for (const FCFHUDAlertItem& ExistingAlert : Alerts)
	{
		if (ExistingAlert.AlertKey == AlertKey)
		{
			return;
		}
	}

	// [v1.0.0] 실제 Gameplay 상태 하나에서 생성한 새 Alert 항목입니다.
	FCFHUDAlertItem NewAlert;
	NewAlert.AlertKey = AlertKey;
	NewAlert.Priority = Priority;
	NewAlert.Title = Title;
	NewAlert.Detail = Detail;
	Alerts.Add(NewAlert);

	Alerts.Sort([](const FCFHUDAlertItem& Left, const FCFHUDAlertItem& Right)
	{
		return static_cast<uint8>(Left.Priority) > static_cast<uint8>(Right.Priority);
	});
}
