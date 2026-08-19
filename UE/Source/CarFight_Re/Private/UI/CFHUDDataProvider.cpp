// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-08-19
// Description: CF-FQ-032 HUD Provider + Player-facing Weapon Selection + actual Weapon Charge/Heat + explicit RPM source + Sensor Snapshot Integration 구현
// Scope: Current Pawn Rebind, Gameplay·Ammo·Charge·Heat·Applied Fitting Weapon Selection 상태와 Actor-free Sensor Snapshot을 10Hz Game-Time ViewData로 변환합니다.
// Changelog:
// - v1.13.0: UI-P0-06 활성 WeaponComp의 실제 WeaponCharge Runtime Current/Maximum/Ratio/Insufficient를 Weapon ViewData에 연결. VehicleBattery나 정적 설정 fallback 없음.
// - v1.12.0: WeaponComp의 Applied Fitting 고정 표시 순서와 SelectedWeaponIndex를 HUD에 연결. 선택 항목은 EquipmentPresetData.DisplayName만 전달하며 내부 MountProfileId/WeaponId/AssetName 노출 0.
// - v1.11.0: UI-P0-06 활성 WeaponComp의 실제 Heat Runtime Current/Maximum/Ratio/Overheated를 Weapon ViewData에 연결. Runtime 비활성은 Unavailable이며 정적 설정 fallback 없음.
// - v1.10.0: UI-P0-06 Current Engine RPM은 계속 Chaos Runtime에서 읽고, RedlineStartRPM/EngineMaxRPM은 Current Pawn VehicleData의 명시 authored 값만 Vehicle ViewData에 전달. 0 Redline은 Unavailable, 추정/보정 없음.
// - v1.9.0: UI-P0-06 호환되는 활성 EquipmentPresetData의 비어 있지 않은 DisplayName만 Player-facing Weapon 이름으로 연결. 내부 ID/AssetName fallback은 사용하지 않음.
// - v1.8.0: UI-P0-06 기존 실제 Ammo·Reserve·Cooldown·Reload·LauncherSequence 필드에서 additive ResourceChannels를 생성. 기존 개별 필드와 Production Presenter는 변경하지 않음.
// - v1.7.0: UI-P0-06에서 VehicleDriveComp가 보유한 실제 UE 5.8 Chaos Movement의 Engine RPM과 Current Gear를 Vehicle ViewData에 연결. 속도 기반 RPM/기어 추정은 사용하지 않음.
// - v1.6.0: TargetSelect 선택/TrackState와 Sensor Snapshot Knowledge를 분리 합성하고 Radar Contact를 Snapshot 기반 상대 위치·거리와 lifecycle data로 변환.
// - v1.5.0: Launcher 상태 이벤트 전용 LauncherSequenceRevision을 ViewData에 전달해 공통 Presentation lifecycle이 Ammo/Timer Refresh와 실제 Sequence 전이를 구분하도록 교정.
// - v1.4.0: finite Ammo Snapshot의 MagazineCapacity를 HUD ViewData로 전달해 Loaded/Capacity 표시 계약을 지원.
// - v1.3.0: VehicleAmmoComp 이벤트를 Pawn Rebind 수명에 연결하고 실제 finite Ammo·Reload Snapshot을 Weapon ViewData로 변환.
// - v1.2.0: 정상 Ripple/Salvo 진행은 Weapon ViewData만 소유하도록 정리하고 AlertFeed용 LauncherSequence 상태 Alert 생성을 제거.
// - v1.1.0: Launcher Sequence 상태 이벤트를 Pawn Rebind 수명에 구독해 Weapon/Alert ViewData를 즉시 갱신.
// - v1.0.0: Vehicle/Weapon/Defense/Target/Alert 실제 Runtime 변환과 Radar/Ammo/Heat/RPM/Gear Unavailable 계약을 구현.
// Migration:
// - v1.13.0 WeaponCharge는 UCFVehicleWeaponComp의 실제 per-weapon Runtime만 읽으며 WeaponData 정적 설정, VehicleBattery, Cooldown에서 현재 Charge를 계산하지 않습니다. VehicleBattery는 계속 Unavailable입니다.
// - v1.12.0 Weapon Selection 목록 순서는 Applied Fitting ResolvedMounts에서 WeaponComp가 보존한 고정 순서를 그대로 사용합니다. Provider는 표시 순번과 DisplayName/Selected만 공개하고 내부 mount identity를 ViewData에 넣지 않습니다.
// - v1.11.0 Heat는 UCFVehicleWeaponComp의 실제 per-weapon Runtime만 읽으며 HeatPerShot/MaxHeat 정적 설정으로 현재 Heat를 계산하지 않습니다.
// - Gameplay 계산을 변경하지 않으며 모든 값은 기존 public Getter/Event/Snapshot의 읽기 전용 소비입니다.
// - TargetSelect는 선택 기록·유효성·TrackState만 제공하고 Relation/Category/InformationLevel/Identity/거리와 Radar Contact는 FCFSensorSnapshot만 제공합니다.
// - 선택 Actor는 UCFVehicleSensorComp::TryGetContactIdForActor로 ContactId만 연결하며 Actor metadata/현재 위치를 HUD data로 읽지 않습니다.
// - Radar Range/Zoom 계약이 없으므로 Snapshot 상대 위치·거리는 제공하지만 NormalizedPosition은 추정하지 않습니다.
// - finite Ammo Runtime Snapshot이 없으면 AmmoAvailability를 Unavailable로 유지하며 MagazineSize나 MaximumLoadableAmmoCount를 현재 탄약으로 사용하지 않습니다.
// - Widget 또는 Presenter에서 Gameplay Actor/Component를 찾지 않습니다.

#include "UI/CFHUDDataProvider.h"

#include "CFEquipmentPresetData.h"
#include "CFLauncherComp.h"
#include "CFVehicleData.h"
#include "CFTargetSelectComp.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleDriveComp.h"
#include "CFVehicleWeaponComp.h"
#include "CFVehicleSensorComp.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/World.h"
#include "UI/CFUISubsystem.h"

namespace
{
	// [v1.6.0] HUD adapter가 Snapshot 위치 데이터의 NaN/Inf를 UI ViewData로 전달하지 않도록 유한 벡터인지 확인합니다.
	bool IsFiniteHUDSensorVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	// [v1.6.0] 안정 ContactId로 Actor-free Snapshot Contact를 찾아 읽기 전용 포인터를 반환합니다.
	const FCFSensorContact* FindHUDSensorContactById(const FCFSensorSnapshot& SensorSnapshot, const FName ContactId)
	{
		if (ContactId.IsNone())
		{
			return nullptr;
		}

		for (const FCFSensorContact& SensorContact : SensorSnapshot.Contacts)
		{
			if (SensorContact.ContactId == ContactId)
			{
				return &SensorContact;
			}
		}
		return nullptr;
	}

	// [v1.6.0] Snapshot origin/forward와 Contact 마지막 신뢰 위치만 사용해 전방(+X)·우측(+Y) 실제 상대 위치 m를 계산합니다.
	bool BuildHUDSensorRelativePositionMeters(
		const FCFSensorSnapshot& SensorSnapshot,
		const FCFSensorContact& SensorContact,
		FVector2D& OutRelativePositionMeters,
		float& OutDistanceMeters)
	{
		OutRelativePositionMeters = FVector2D::ZeroVector;
		OutDistanceMeters = 0.0f;
		if (!IsFiniteHUDSensorVector(SensorSnapshot.SensorOriginWorldLocation)
			|| !IsFiniteHUDSensorVector(SensorSnapshot.SensorForwardWorldDirection)
			|| !IsFiniteHUDSensorVector(SensorContact.LastKnownWorldLocation))
		{
			return false;
		}

		// [v1.6.0] 차량 Radar 평면의 전방 축으로 사용할 Snapshot Forward의 XY 정규화 값입니다.
		FVector PlanarForwardDirection(
			SensorSnapshot.SensorForwardWorldDirection.X,
			SensorSnapshot.SensorForwardWorldDirection.Y,
			0.0f);
		PlanarForwardDirection = PlanarForwardDirection.GetSafeNormal();
		if (PlanarForwardDirection.IsNearlyZero())
		{
			return false;
		}

		// [v1.6.0] UE +Z World Up과 Snapshot 전방으로 만든 차량 Radar 평면의 우측 축입니다.
		const FVector PlanarRightDirection = FVector::CrossProduct(FVector::UpVector, PlanarForwardDirection).GetSafeNormal();
		if (PlanarRightDirection.IsNearlyZero())
		{
			return false;
		}

		// [v1.6.0] Sensor 기준점에서 마지막 신뢰 Contact 위치까지의 실제 World offset입니다.
		const FVector RelativeWorldOffset = SensorContact.LastKnownWorldLocation - SensorSnapshot.SensorOriginWorldLocation;

		// [v1.6.0] Radar 평면 전방 방향의 실제 상대 거리 m입니다.
		const float ForwardMeters = static_cast<float>(FVector::DotProduct(RelativeWorldOffset, PlanarForwardDirection) / 100.0);

		// [v1.6.0] Radar 평면 우측 방향의 실제 상대 거리 m입니다.
		const float RightMeters = static_cast<float>(FVector::DotProduct(RelativeWorldOffset, PlanarRightDirection) / 100.0);

		// [v1.6.0] Sensor 기준점에서 마지막 신뢰 Contact 위치까지의 3D 실제 거리 m입니다.
		const float DistanceMeters = static_cast<float>(RelativeWorldOffset.Size() / 100.0);
		if (!FMath::IsFinite(ForwardMeters) || !FMath::IsFinite(RightMeters) || !FMath::IsFinite(DistanceMeters))
		{
			return false;
		}

		OutRelativePositionMeters = FVector2D(ForwardMeters, RightMeters);
		OutDistanceMeters = FMath::Max(0.0f, DistanceMeters);
		return true;
	}
}

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

	// [v1.7.0] 현재 차량의 실제 Chaos Vehicle Movement를 소유하는 Drive Runtime입니다.
	const UCFVehicleDriveComp* VehicleDriveComponent = VehiclePawn->GetVehicleDriveComp();
	// [v1.7.0] Engine RPM과 실제 변속 단수를 제공하는 UE 5.8 Chaos Movement입니다.
	const UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = VehicleDriveComponent
		? VehicleDriveComponent->GetVehicleMovementComponent()
		: nullptr;
		if (VehicleMovementComponent)
	{
		// [v1.7.0] Chaos Mechanical Simulation이 제공하는 실제 현재 Engine RPM입니다.
		const float CurrentEngineRpm = VehicleMovementComponent->GetEngineRotationSpeed();
		if (FMath::IsFinite(CurrentEngineRpm))
		{
			OutVehicleViewData.EngineRpm = FMath::Max(0.0f, CurrentEngineRpm);
			OutVehicleViewData.EngineRpmAvailability = ResolveKnownNumericAvailability(true, OutVehicleViewData.EngineRpm);
		}

		// [v1.10.0] Chaos EngineSetup.MaxRPM의 실제 source와 HUD Redline authored source를 함께 가진 현재 VehicleData입니다.
		const UCFVehicleData* VehicleData = VehiclePawn->VehicleData;
		if (VehicleData)
		{
			// [v1.10.0] 실제 Chaos EngineSetup.MaxRPM으로 적용되는 물리 엔진 최대 RPM입니다.
			const float MaximumEngineRpm = VehicleData->VehicleMovementConfig.EngineMaxRPM;
			if (FMath::IsFinite(MaximumEngineRpm) && MaximumEngineRpm > KINDA_SMALL_NUMBER)
			{
				OutVehicleViewData.EngineMaximumRpm = MaximumEngineRpm;
				OutVehicleViewData.EngineMaximumRpmAvailability = ECFUIViewAvailability::Known;
			}

			// [v1.10.0] EngineMaxRPM이나 변속 설정에서 추정하지 않고 VehicleData에 명시된 값만 사용하는 레드라인 시작 RPM입니다.
			const float RedlineStartRpm = VehicleData->VehicleMovementConfig.RedlineStartRPM;
			if (FMath::IsFinite(RedlineStartRpm) && RedlineStartRpm > KINDA_SMALL_NUMBER)
			{
				OutVehicleViewData.EngineRedlineStartRpm = RedlineStartRpm;
				OutVehicleViewData.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Known;
			}
		}

		// [v1.7.0] Chaos Transmission이 제공하는 실제 현재 Gear입니다. 음수=Reverse, 0=Neutral, 1+=Forward 계약을 표시 Text로만 변환합니다.
		const int32 CurrentGear = VehicleMovementComponent->GetCurrentGear();
		OutVehicleViewData.GearAvailability = ECFUIViewAvailability::Known;
		if (CurrentGear < 0)
		{
			OutVehicleViewData.GearText = FText::FromString(TEXT("R"));
		}
		else if (CurrentGear == 0)
		{
			OutVehicleViewData.GearText = FText::FromString(TEXT("N"));
		}
		else
		{
			OutVehicleViewData.GearText = FText::AsNumber(CurrentGear);
		}
	}

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

			// [v1.12.0] 현재 활성 무기와 Applied Fitting 기반 선택 목록을 함께 소유하는 Weapon Runtime입니다.
	const UCFVehicleWeaponComp* WeaponComponent = VehiclePawn->GetVehicleWeaponComp();
	if (WeaponComponent && WeaponComponent->HasWeaponSelectionRuntime())
	{
		// [v1.12.0] Applied Fitting 고정 표시 순서에서 실제 선택 가능 무기 수입니다.
		const int32 SelectableWeaponCount = WeaponComponent->GetSelectableWeaponCount();
		OutWeaponViewData.WeaponSelectionAvailability = SelectableWeaponCount > 0
			? ECFUIViewAvailability::Known
			: ECFUIViewAvailability::KnownZero;
		OutWeaponViewData.SelectedWeaponIndex = WeaponComponent->GetSelectedWeaponIndex();
		OutWeaponViewData.SelectableWeapons.Reserve(SelectableWeaponCount);

		for (int32 WeaponIndex = 0; WeaponIndex < SelectableWeaponCount; ++WeaponIndex)
		{
			// [v1.12.0] 내부 identity 없이 Player-facing DisplayName 가용 상태와 선택 여부만 담는 HUD 항목입니다.
			FCFWeaponSelectionHUDItem SelectionHUDItem;
			SelectionHUDItem.bSelected = WeaponIndex == OutWeaponViewData.SelectedWeaponIndex;
			if (WeaponComponent->IsSelectableWeaponDisplayNameAvailable(WeaponIndex))
			{
				SelectionHUDItem.DisplayNameAvailability = ECFUIViewAvailability::Known;
				SelectionHUDItem.DisplayName = WeaponComponent->GetSelectableWeaponDisplayName(WeaponIndex);
			}
			OutWeaponViewData.SelectableWeapons.Add(SelectionHUDItem);
		}
	}

	if (!WeaponDebug.ActiveWeaponData)
	{
		OutWeaponViewData.Availability = ECFUIViewAvailability::KnownZero;
	}
	else
	{
		OutWeaponViewData.Availability = ECFUIViewAvailability::Known;
		OutWeaponViewData.WeaponId = WeaponDebug.ActiveWeaponId;

		// [v1.9.0] 현재 활성 무기와 같은 Runtime 선택에서 해석된 EquipmentPresetData Source입니다.
		// [v1.9.0] Player-facing 이름 후보를 제공할 현재 호환 활성 EquipmentPresetData입니다.
		const UCFEquipmentPresetData* ActiveEquipmentPresetData = WeaponComponent
			? WeaponComponent->GetActiveEquipmentPresetData()
			: nullptr;
		if (ActiveEquipmentPresetData
			&& WeaponComponent->IsActiveEquipmentPresetCompatible()
			&& !ActiveEquipmentPresetData->DisplayName.IsEmpty())
		{
			OutWeaponViewData.DisplayNameAvailability = ECFUIViewAvailability::Known;
			OutWeaponViewData.DisplayName = ActiveEquipmentPresetData->DisplayName;
		}
		else
		{
			OutWeaponViewData.DisplayNameAvailability = ECFUIViewAvailability::Unavailable;
			OutWeaponViewData.DisplayName = FText::GetEmpty();
		}

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

							// [v1.13.0] 실제 per-weapon WeaponCharge Runtime을 같은 현재 WeaponComp에서 읽습니다.
	const UCFVehicleWeaponComp* ChargeWeaponComponent = WeaponComponent;
	if (ChargeWeaponComponent && ChargeWeaponComponent->IsActiveWeaponChargeRuntimeEnabled())
	{
		OutWeaponViewData.CurrentWeaponCharge = FMath::Max(ChargeWeaponComponent->GetCurrentWeaponCharge(), 0.0f);
		OutWeaponViewData.MaximumWeaponCharge = FMath::Max(ChargeWeaponComponent->GetActiveWeaponMaximumCharge(), 0.0f);
		OutWeaponViewData.WeaponChargeRatio = FMath::Clamp(ChargeWeaponComponent->GetActiveWeaponChargeRatio(), 0.0f, 1.0f);
		OutWeaponViewData.bWeaponChargeInsufficient = !ChargeWeaponComponent->CanActiveWeaponAcceptChargeShot();
		OutWeaponViewData.WeaponChargeAvailability = ResolveKnownNumericAvailability(true, OutWeaponViewData.CurrentWeaponCharge);
	}
	else
	{
		OutWeaponViewData.WeaponChargeAvailability = ECFUIViewAvailability::Unavailable;
	}

						// [v1.12.0] 실제 per-weapon Heat Runtime을 같은 현재 WeaponComp에서 읽습니다.
	const UCFVehicleWeaponComp* HeatWeaponComponent = WeaponComponent;
	if (HeatWeaponComponent && HeatWeaponComponent->IsActiveWeaponHeatRuntimeEnabled())
	{
		OutWeaponViewData.CurrentHeat = FMath::Max(HeatWeaponComponent->GetCurrentWeaponHeat(), 0.0f);
		OutWeaponViewData.MaximumHeat = FMath::Max(HeatWeaponComponent->GetActiveWeaponMaximumHeat(), 0.0f);
		OutWeaponViewData.HeatRatio = FMath::Clamp(HeatWeaponComponent->GetActiveWeaponHeatRatio(), 0.0f, 1.0f);
		OutWeaponViewData.bWeaponOverheated = HeatWeaponComponent->IsActiveWeaponOverheated();
		OutWeaponViewData.HeatAvailability = ResolveKnownNumericAvailability(true, OutWeaponViewData.CurrentHeat);
	}
	else
	{
		OutWeaponViewData.HeatAvailability = ECFUIViewAvailability::Unavailable;
	}

	UCFLauncherComp* LauncherComponent = VehiclePawn->GetLauncherComp();
	if (LauncherComponent)
	{
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

	// [v1.8.0] 기존 실제 Weapon 필드를 additive 공통 Resource Channel 목록으로 투영합니다.
	OutWeaponViewData.RebuildResourceChannelsFromCurrentFields();
}

// [v1.6.0] TargetSelect의 선택/TrackState와 Sensor Snapshot의 Player Knowledge를 중복 판정 없이 합성합니다.
void UCFHUDDataProvider::FillTargetViewData(FCFTargetHUDData& OutTargetViewData) const
{
	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	// [v1.6.0] 현재 선택 기록·유효성·TrackState를 소유하는 TargetSelect Runtime입니다.
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

	OutTargetViewData.Availability = ECFUIViewAvailability::Known;
	OutTargetViewData.bHasSelectedTarget = true;
	OutTargetViewData.bSelectedTargetValid = TargetSelectComponent->IsSelectedTargetValid();
	OutTargetViewData.TrackState = TargetSelectComponent->GetSelectedTargetTrackState();

	// [v1.6.0] Target Knowledge와 Radar Contact의 Player-facing source인 현재 차량 Sensor Runtime입니다.
	UCFVehicleSensorComp* SensorComponent = VehiclePawn->GetVehicleSensorComp();
	if (!SensorComponent)
	{
		OutTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Unavailable;
		return;
	}

	// [v1.6.0] Target Knowledge를 읽을 유일한 Actor-free Sensor Snapshot 사본입니다.
	const FCFSensorSnapshot SensorSnapshot = SensorComponent->GetSensorSnapshot();
	if (!SensorSnapshot.bRuntimeReady || !SensorSnapshot.IsPublicContractValid())
	{
		OutTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Unavailable;
		return;
	}

	// [v1.6.0] 선택 owner에서 현재 선택 기록과 연결된 Actor 참조만 가져오며 metadata/현재 위치는 읽지 않습니다.
	AActor* SelectedTargetActor = TargetSelectComponent->GetSelectedTargetActor();
	if (!IsValid(SelectedTargetActor))
	{
		OutTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Unknown;
		return;
	}

	// [v1.6.0] 선택 Actor를 Sensor private runtime data가 아닌 공개 Snapshot Contact에 연결할 안정 ID입니다.
	FName SelectedContactId = NAME_None;
	if (!SensorComponent->TryGetContactIdForActor(SelectedTargetActor, SelectedContactId))
	{
		OutTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Unknown;
		return;
	}

	// [v1.6.0] Player-facing Knowledge를 실제로 읽을 선택 대상의 Actor-free Snapshot Contact입니다.
	const FCFSensorContact* SelectedSensorContact = FindHUDSensorContactById(SensorSnapshot, SelectedContactId);
	if (!SelectedSensorContact)
	{
		OutTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Unknown;
		return;
	}

	OutTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Known;
	OutTargetViewData.ContactId = SelectedSensorContact->ContactId;
	OutTargetViewData.TargetId = SelectedSensorContact->KnownTargetId;
	OutTargetViewData.Relation = SelectedSensorContact->Relation;
	OutTargetViewData.Category = SelectedSensorContact->TargetCategory;
	OutTargetViewData.InformationLevel = SelectedSensorContact->InformationLevel;
	OutTargetViewData.ContactState = SelectedSensorContact->ContactState;
	OutTargetViewData.FreshnessSeconds = SelectedSensorContact->FreshnessSeconds;
	OutTargetViewData.AnalysisProgress01 = SelectedSensorContact->AnalysisProgress01;
	OutTargetViewData.bDestroyedConfirmed = SelectedSensorContact->bDestroyedConfirmed;

	// [v1.6.0] Identity는 Sensor Knowledge가 KnownTargetId와 KnownDisplayName을 공개한 경우에만 HUD Known으로 승격합니다.
	const bool bIdentityKnown = !SelectedSensorContact->KnownTargetId.IsNone()
		&& !SelectedSensorContact->KnownDisplayName.IsEmpty()
		&& (SelectedSensorContact->InformationLevel == ECFTargetInfoLevel::Identified
			|| SelectedSensorContact->InformationLevel == ECFTargetInfoLevel::DetailedScan);
	if (bIdentityKnown)
	{
		OutTargetViewData.IdentityAvailability = ECFUIViewAvailability::Known;
		OutTargetViewData.DisplayName = SelectedSensorContact->KnownDisplayName;
	}
	else
	{
		OutTargetViewData.IdentityAvailability = ECFUIViewAvailability::Unknown;
	}

	// [v1.6.0] Target 거리 역시 Actor 현재 위치가 아니라 Snapshot origin과 마지막 신뢰 위치만으로 계산합니다.
	FVector2D RelativePositionMeters;
	float DistanceMeters = 0.0f;
	if (BuildHUDSensorRelativePositionMeters(SensorSnapshot, *SelectedSensorContact, RelativePositionMeters, DistanceMeters))
	{
		OutTargetViewData.DistanceMeters = DistanceMeters;
		OutTargetViewData.DistanceAvailability = ResolveKnownNumericAvailability(true, DistanceMeters);
	}
}

// [v1.6.0] Actor-free Sensor Snapshot Contact를 Radar ViewData로 읽기 전용 변환합니다.
void UCFHUDDataProvider::FillRadarViewData(FCFRadarHUDData& OutRadarViewData) const
{
	OutRadarViewData.Contacts.Reset();

	ACFVehiclePawn* VehiclePawn = BoundVehiclePawn.Get();
	if (!VehiclePawn)
	{
		return;
	}

	// [v1.6.0] Radar Contact의 유일한 Gameplay source인 현재 차량 Sensor Runtime입니다.
	UCFVehicleSensorComp* SensorComponent = VehiclePawn->GetVehicleSensorComp();
	if (!SensorComponent)
	{
		return;
	}

	// [v1.6.0] Radar adapter가 읽을 Actor-free Sensor Snapshot 사본입니다.
	const FCFSensorSnapshot SensorSnapshot = SensorComponent->GetSensorSnapshot();
	if (!SensorSnapshot.bRuntimeReady || !SensorSnapshot.IsPublicContractValid())
	{
		return;
	}

	OutRadarViewData.Availability = SensorSnapshot.Contacts.IsEmpty()
		? ECFUIViewAvailability::KnownZero
		: ECFUIViewAvailability::Known;

	// [v1.6.0] Radar의 bSelected 표시만 연결할 현재 선택 ContactId입니다. 선택 자체는 TargetSelect가 계속 소유합니다.
	FName SelectedContactId = NAME_None;
	UCFTargetSelectComp* TargetSelectComponent = VehiclePawn->GetTargetSelectComp();
	if (TargetSelectComponent && TargetSelectComponent->HasSelectedTarget())
	{
		// [v1.6.0] ContactId 연결에만 사용할 현재 선택 Actor입니다. Actor metadata나 위치는 읽지 않습니다.
		AActor* SelectedTargetActor = TargetSelectComponent->GetSelectedTargetActor();
		if (IsValid(SelectedTargetActor))
		{
			SensorComponent->TryGetContactIdForActor(SelectedTargetActor, SelectedContactId);
		}
	}

	OutRadarViewData.Contacts.Reserve(SensorSnapshot.Contacts.Num());
	for (const FCFSensorContact& SensorContact : SensorSnapshot.Contacts)
	{
		// [v1.6.0] Snapshot Contact 한 건을 Gameplay 재판정 없이 복사·좌표 변환할 Radar ViewData 항목입니다.
		FCFRadarContactHUDData RadarContact;
		RadarContact.ContactId = SensorContact.ContactId;
		RadarContact.Relation = SensorContact.Relation;
		RadarContact.Category = SensorContact.TargetCategory;
		RadarContact.InformationLevel = SensorContact.InformationLevel;
		RadarContact.ContactState = SensorContact.ContactState;
		RadarContact.FreshnessSeconds = SensorContact.FreshnessSeconds;
		RadarContact.AnalysisProgress01 = SensorContact.AnalysisProgress01;
		RadarContact.bDestroyedConfirmed = SensorContact.bDestroyedConfirmed;
		RadarContact.bSelected = !SelectedContactId.IsNone() && SensorContact.ContactId == SelectedContactId;
		RadarContact.NormalizedPositionAvailability = ECFUIViewAvailability::Unavailable;

		// [v1.6.0] Radar Range/Zoom을 추정하지 않고 Snapshot 기반 실제 상대 위치·거리만 계산합니다.
		FVector2D RelativePositionMeters;
		float DistanceMeters = 0.0f;
		if (BuildHUDSensorRelativePositionMeters(SensorSnapshot, SensorContact, RelativePositionMeters, DistanceMeters))
		{
			RadarContact.RelativePositionAvailability = ECFUIViewAvailability::Known;
			RadarContact.RelativePositionMeters = RelativePositionMeters;
			RadarContact.DistanceMeters = DistanceMeters;
		}

		OutRadarViewData.Contacts.Add(RadarContact);
	}
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
