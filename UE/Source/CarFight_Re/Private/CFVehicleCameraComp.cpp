// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.1.0
// Date: 2026-04-10
// Description: CarFight 차량 카메라 컴포넌트 구현 초안
// Scope: 차량 중심 피벗 기반 자유 조준, 제한각 Clamp, SpringArm 연동, Aim Trace 계산 골격을 구현합니다.

#include "CFVehicleCameraComp.h"

#include "CFVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"

namespace
{
	// Owner Actor에서 이름으로 원하는 컴포넌트를 찾습니다.
	template<typename ComponentType>
	ComponentType* FindComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		TArray<ComponentType*> FoundComponents;
		OwnerActor->GetComponents<ComponentType>(FoundComponents);
		for (ComponentType* FoundComponent : FoundComponents)
		{
			if (FoundComponent && FoundComponent->GetFName() == ComponentName)
			{
				return FoundComponent;
			}
		}

		return nullptr;
	}
}

// [v0.1.0] 기본 생성자
UCFVehicleCameraComp::UCFVehicleCameraComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// [v0.1.0] BeginPlay에서 카메라 런타임 초기화를 시도합니다.
void UCFVehicleCameraComp::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitializeOnBeginPlay)
	{
		InitializeCameraRuntime();
	}
}

// [v0.1.0] Tick에서 차량 상태를 읽어 카메라를 갱신합니다.
void UCFVehicleCameraComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bCameraRuntimeReady && !InitializeCameraRuntime())
	{
		return;
	}

	const FCFVehicleCameraTuningConfig LocalCameraTuningConfig = VehicleCameraData ? VehicleCameraData->CameraTuningConfig : FCFVehicleCameraTuningConfig();
	const FCFVehicleCameraAimProfile LocalAimProfile = BuildResolvedAimProfile();

	CameraRuntimeState.PreviousCameraMode = CameraRuntimeState.CurrentCameraMode;
	CameraRuntimeState.CurrentCameraMode = EvaluateCameraMode();
	CameraRuntimeState.bCameraModeChangedThisFrame = CameraRuntimeState.PreviousCameraMode != CameraRuntimeState.CurrentCameraMode;
	CameraRuntimeState.ActiveAimProfileName = LocalAimProfile.ProfileName;

	UpdateAimState(DeltaTime, LocalCameraTuningConfig, LocalAimProfile);
	UpdateCameraTransform(DeltaTime, LocalCameraTuningConfig, LocalAimProfile);
	UpdateAimTrace(LocalCameraTuningConfig);
}

// [v0.1.0] 카메라 참조 검색과 기본 상태 설정을 다시 시도합니다.
bool UCFVehicleCameraComp::InitializeCameraRuntime()
{
	ResolveVehiclePawnOwner();
	const bool bResolvedReferences = ResolveCameraReferences();
	if (!bResolvedReferences)
	{
		bCameraRuntimeReady = false;
		return false;
	}

	const FCFVehicleCameraTuningConfig LocalCameraTuningConfig = VehicleCameraData ? VehicleCameraData->CameraTuningConfig : FCFVehicleCameraTuningConfig();
	CurrentArmLength = LocalCameraTuningConfig.BaseArmLength;
	CurrentFOV = LocalCameraTuningConfig.BaseFOV;
	CameraRuntimeState.CurrentArmLength = CurrentArmLength;
	CameraRuntimeState.CurrentFOV = CurrentFOV;
	bCameraRuntimeReady = true;
	return true;
}

// [v0.1.0] 현재 프레임 Look 입력값을 교체합니다.
void UCFVehicleCameraComp::SetLookInput(FVector2D InLookInput)
{
	PendingLookInput = InLookInput;
}

// [v0.1.0] 현재 프레임 Look 입력값에 추가 입력을 누적합니다.
void UCFVehicleCameraComp::AddLookInput(float InLookInputX, float InLookInputY)
{
	PendingLookInput.X += InLookInputX;
	PendingLookInput.Y += InLookInputY;
}

// [v0.1.0] 현재 프레임 Look 입력을 0으로 초기화합니다.
void UCFVehicleCameraComp::ClearLookInput()
{
	PendingLookInput = FVector2D::ZeroVector;
}

// [v0.1.0] 현재 조준을 차량 정면 기준으로 되돌립니다.
void UCFVehicleCameraComp::ResetAimToVehicleForward()
{
	AccumulatedAimYaw = 0.0f;
	AccumulatedAimPitch = 0.0f;
	CameraRuntimeState.AccumulatedAimYaw = 0.0f;
	CameraRuntimeState.AccumulatedAimPitch = 0.0f;
	CameraRuntimeState.ClampedAimYaw = 0.0f;
	CameraRuntimeState.ClampedAimPitch = 0.0f;
}

// [v0.1.0] 외부 시스템이 카메라 모드 Modifier 플래그를 한 번에 갱신합니다.
void UCFVehicleCameraComp::SetCameraModeFlags(const FCFVehicleCameraModeFlags& InCameraModeFlags)
{
	CameraModeFlags = InCameraModeFlags;
}

// [v0.1.0] 외부 시스템이 임시 Aim Profile을 주입합니다.
void UCFVehicleCameraComp::SetAimProfileOverride(const FCFVehicleCameraAimProfile& InAimProfile)
{
	AimProfileOverride = InAimProfile;
	bUseAimProfileOverride = true;
}

// [v0.1.0] 임시 Aim Profile 주입 상태를 해제합니다.
void UCFVehicleCameraComp::ClearAimProfileOverride()
{
	AimProfileOverride = FCFVehicleCameraAimProfile();
	bUseAimProfileOverride = false;
}

// [v0.1.0] 현재 카메라 런타임 스냅샷을 반환합니다.
FCFVehicleCameraRuntimeState UCFVehicleCameraComp::GetCameraRuntimeState() const
{
	return CameraRuntimeState;
}

// [v0.1.0] 현재 해석된 Aim Profile을 반환합니다.
FCFVehicleCameraAimProfile UCFVehicleCameraComp::GetResolvedAimProfile() const
{
	return BuildResolvedAimProfile();
}

// [v0.1.0] 현재 Clamp 적용 후 Aim Rotation을 반환합니다.
FRotator UCFVehicleCameraComp::GetCurrentAimRotation() const
{
	return FRotator(CameraRuntimeState.ClampedAimPitch, CameraRuntimeState.ClampedAimYaw, 0.0f);
}

// [v0.1.0] 현재 카메라가 향하는 월드 방향 벡터를 반환합니다.
FVector UCFVehicleCameraComp::GetCurrentAimDirection() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetForwardVector();
	}

	const FRotator BaseVehicleAimRotation = GetBaseVehicleAimRotation();
	const FRotator WorldAimRotation(
		BaseVehicleAimRotation.Pitch + CameraRuntimeState.ClampedAimPitch,
		BaseVehicleAimRotation.Yaw + CameraRuntimeState.ClampedAimYaw,
		0.0f);
	return WorldAimRotation.Vector();
}

// [v0.1.0] 현재 Aim Trace 적중 위치를 반환합니다.
FVector UCFVehicleCameraComp::GetCurrentAimHitLocation() const
{
	return CameraRuntimeState.AimHitLocation;
}

// [v0.1.0] Owner가 ACFVehiclePawn인지 확인하고 캐시합니다.
ACFVehiclePawn* UCFVehicleCameraComp::ResolveVehiclePawnOwner()
{
	if (CachedVehiclePawn.IsValid())
	{
		return CachedVehiclePawn.Get();
	}

	ACFVehiclePawn* OwnerVehiclePawn = Cast<ACFVehiclePawn>(GetOwner());
	CachedVehiclePawn = OwnerVehiclePawn;
	return OwnerVehiclePawn;
}

// [v0.1.0] Owner에서 필요한 카메라 컴포넌트 참조를 자동으로 검색합니다.
bool UCFVehicleCameraComp::ResolveCameraReferences()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (!CameraPivotRoot)
	{
		CameraPivotRoot = FindComponentByName<USceneComponent>(OwnerActor, DefaultPivotRootName);
	}

	if (!CameraAimPivot)
	{
		CameraAimPivot = FindComponentByName<USceneComponent>(OwnerActor, DefaultAimPivotName);
	}

	if (!CameraBoom)
	{
		CameraBoom = FindComponentByName<USpringArmComponent>(OwnerActor, DefaultCameraBoomName);
	}

	if (!FollowCamera)
	{
		FollowCamera = FindComponentByName<UCameraComponent>(OwnerActor, DefaultFollowCameraName);
	}

	return CameraBoom != nullptr || FollowCamera != nullptr;
}

// [v0.1.0] 현재 카메라 모드를 평가합니다.
ECFVehicleCameraMode UCFVehicleCameraComp::EvaluateCameraMode() const
{
	if (CameraModeFlags.bDestroyed)
	{
		return ECFVehicleCameraMode::Destroyed;
	}

	if (CameraModeFlags.bSpectate)
	{
		return ECFVehicleCameraMode::Spectate;
	}

	if (CameraModeFlags.bReverse)
	{
		return ECFVehicleCameraMode::Reverse;
	}

	if (CameraModeFlags.bAirborne)
	{
		return ECFVehicleCameraMode::Airborne;
	}

	if (CameraModeFlags.bCombat)
	{
		return ECFVehicleCameraMode::Combat;
	}

	return ECFVehicleCameraMode::Normal;
}

// [v0.1.0] 현재 사용할 Aim Profile을 조합해 반환합니다.
FCFVehicleCameraAimProfile UCFVehicleCameraComp::BuildResolvedAimProfile() const
{
	if (bUseAimProfileOverride)
	{
		return AimProfileOverride;
	}

	if (VehicleCameraData)
	{
		return VehicleCameraData->DefaultAimProfile;
	}

	return FCFVehicleCameraAimProfile();
}

// [v0.1.0] 현재 차량 속도(km/h)를 읽어옵니다.
float UCFVehicleCameraComp::GetVehicleSpeedKmh() const
{
	const ACFVehiclePawn* OwnerVehiclePawn = CachedVehiclePawn.IsValid() ? CachedVehiclePawn.Get() : Cast<ACFVehiclePawn>(GetOwner());
	if (!OwnerVehiclePawn)
	{
		return 0.0f;
	}

	return OwnerVehiclePawn->GetVehicleSpeed();
}

// [v0.1.0] 차량 기준 기본 Aim Rotation을 계산합니다.
FRotator UCFVehicleCameraComp::GetBaseVehicleAimRotation() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FRotator::ZeroRotator;
	}

	if (CameraPivotRoot)
	{
		const FRotator PivotWorldRotation = CameraPivotRoot->GetComponentRotation();
		return FRotator(0.0f, PivotWorldRotation.Yaw, 0.0f);
	}

	const FRotator OwnerActorRotation = OwnerActor->GetActorRotation();
	return FRotator(0.0f, OwnerActorRotation.Yaw, 0.0f);
}

// [v0.1.0] 카메라 피벗의 월드 위치를 계산합니다.
FVector UCFVehicleCameraComp::GetPivotWorldLocation(const FCFVehicleCameraTuningConfig& CameraTuningConfig) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FVector::ZeroVector;
	}

	const FRotator BaseVehicleAimRotation = GetBaseVehicleAimRotation();
	const FVector PivotBaseLocation = CameraPivotRoot ? CameraPivotRoot->GetComponentLocation() : OwnerActor->GetActorLocation();
	return PivotBaseLocation + BaseVehicleAimRotation.RotateVector(CameraTuningConfig.PivotLocalOffset);
}

// [v0.1.0] 입력을 누적하고 Aim Yaw/Pitch를 Clamp 합니다.
void UCFVehicleCameraComp::UpdateAimState(float DeltaTime, const FCFVehicleCameraTuningConfig& CameraTuningConfig, const FCFVehicleCameraAimProfile& AimProfile)
{
	const float InputScale = CameraTuningConfig.bScaleLookInputByDeltaTime ? DeltaTime : 1.0f;
	AccumulatedAimYaw += PendingLookInput.X * CameraTuningConfig.LookYawSpeedDegPerSec * InputScale;
	AccumulatedAimPitch += PendingLookInput.Y * CameraTuningConfig.LookPitchSpeedDegPerSec * InputScale;

	CameraRuntimeState.AccumulatedAimYaw = AccumulatedAimYaw;
	CameraRuntimeState.AccumulatedAimPitch = AccumulatedAimPitch;

	const float ClampedAimYaw = FMath::Clamp(AccumulatedAimYaw, AimProfile.MinYawDeg, AimProfile.MaxYawDeg);
	const float ClampedAimPitch = FMath::Clamp(AccumulatedAimPitch, AimProfile.MinPitchDeg, AimProfile.MaxPitchDeg);

	CameraRuntimeState.ClampedAimYaw = ClampedAimYaw;
	CameraRuntimeState.ClampedAimPitch = ClampedAimPitch;

	AccumulatedAimYaw = ClampedAimYaw;
	AccumulatedAimPitch = ClampedAimPitch;

	const float DistanceToYawMin = FMath::Abs(ClampedAimYaw - AimProfile.MinYawDeg);
	const float DistanceToYawMax = FMath::Abs(ClampedAimYaw - AimProfile.MaxYawDeg);
	const float DistanceToPitchMin = FMath::Abs(ClampedAimPitch - AimProfile.MinPitchDeg);
	const float DistanceToPitchMax = FMath::Abs(ClampedAimPitch - AimProfile.MaxPitchDeg);

	CameraRuntimeState.bAimAtYawLimit = DistanceToYawMin <= AimProfile.YawSoftLimitZoneDeg || DistanceToYawMax <= AimProfile.YawSoftLimitZoneDeg;
	CameraRuntimeState.bAimAtPitchLimit = DistanceToPitchMin <= AimProfile.PitchSoftLimitZoneDeg || DistanceToPitchMax <= AimProfile.PitchSoftLimitZoneDeg;
}

// [v0.1.0] 목표 카메라 위치와 FOV를 계산해 SpringArm / Camera에 반영합니다.
void UCFVehicleCameraComp::UpdateCameraTransform(float DeltaTime, const FCFVehicleCameraTuningConfig& CameraTuningConfig, const FCFVehicleCameraAimProfile& AimProfile)
{
	const float VehicleSpeedKmh = FMath::Max(0.0f, GetVehicleSpeedKmh());
	const float SpeedAlpha = CameraTuningConfig.SpeedForMaxBonusKmh > KINDA_SMALL_NUMBER
		? FMath::Clamp(VehicleSpeedKmh / CameraTuningConfig.SpeedForMaxBonusKmh, 0.0f, 1.0f)
		: 0.0f;

	float DesiredArmLength = CameraTuningConfig.BaseArmLength;
	float DesiredFOV = CameraTuningConfig.BaseFOV;
	float ResolvedHeightOffset = CameraTuningConfig.BaseHeightOffset + AimProfile.HeightOffset;
	float ResolvedSideOffset = CameraTuningConfig.BaseSideOffset + AimProfile.SideOffset;

	if (CameraRuntimeState.CurrentCameraMode == ECFVehicleCameraMode::Combat)
	{
		DesiredArmLength += CameraTuningConfig.CombatArmLengthOffset;
		DesiredFOV += CameraTuningConfig.CombatFOVOffset;
	}
	else if (CameraRuntimeState.CurrentCameraMode == ECFVehicleCameraMode::Reverse)
	{
		DesiredArmLength += CameraTuningConfig.ReverseArmLengthOffset;
		DesiredFOV += CameraTuningConfig.ReverseFOVOffset;
	}
	else if (CameraRuntimeState.CurrentCameraMode == ECFVehicleCameraMode::Airborne)
	{
		DesiredFOV += CameraTuningConfig.AirborneFOVOffset;
	}

	if (CameraTuningConfig.bUseSpeedBasedArmLength)
	{
		DesiredArmLength += CameraTuningConfig.MaxSpeedArmLengthBonus * SpeedAlpha;
	}

	if (CameraTuningConfig.bUseSpeedBasedFOV)
	{
		DesiredFOV += CameraTuningConfig.MaxSpeedFOVBonus * SpeedAlpha;
	}

	DesiredArmLength += AimProfile.ArmLengthOffset;
	DesiredFOV += AimProfile.FOVOffset;
	DesiredArmLength = FMath::Max(CameraTuningConfig.MinArmLength, DesiredArmLength);

	CurrentArmLength = FMath::FInterpTo(CurrentArmLength, DesiredArmLength, DeltaTime, CameraTuningConfig.ArmLengthInterpSpeed);
	CurrentFOV = FMath::FInterpTo(CurrentFOV, DesiredFOV, DeltaTime, CameraTuningConfig.FOVInterpSpeed);

	CameraRuntimeState.DesiredArmLength = DesiredArmLength;
	CameraRuntimeState.CurrentArmLength = CurrentArmLength;
	CameraRuntimeState.DesiredFOV = DesiredFOV;
	CameraRuntimeState.CurrentFOV = CurrentFOV;

	const FVector PivotWorldLocation = GetPivotWorldLocation(CameraTuningConfig);
	const FRotator BaseVehicleAimRotation = GetBaseVehicleAimRotation();
	const FRotator WorldAimRotation(
		BaseVehicleAimRotation.Pitch + CameraRuntimeState.ClampedAimPitch,
		BaseVehicleAimRotation.Yaw + CameraRuntimeState.ClampedAimYaw,
		0.0f);

	if (CameraAimPivot)
	{
		CameraAimPivot->SetWorldLocationAndRotation(PivotWorldLocation, WorldAimRotation);
	}

	if (CameraBoom)
	{
		CameraBoom->bUsePawnControlRotation = false;
		CameraBoom->bDoCollisionTest = CameraTuningConfig.bEnableBoomCollisionTest;
		CameraBoom->ProbeChannel = ECC_Camera;
		CameraBoom->ProbeSize = CameraTuningConfig.CollisionProbeSize;
		CameraBoom->SetWorldLocationAndRotation(PivotWorldLocation, WorldAimRotation);
		CameraBoom->TargetArmLength = CurrentArmLength;
		CameraBoom->SocketOffset = FVector(0.0f, ResolvedSideOffset, ResolvedHeightOffset);
	}
	else if (FollowCamera)
	{
		const FVector ManualCameraLocation = PivotWorldLocation - (WorldAimRotation.Vector() * CurrentArmLength) + WorldAimRotation.RotateVector(FVector(0.0f, ResolvedSideOffset, ResolvedHeightOffset));
		FollowCamera->SetWorldLocationAndRotation(ManualCameraLocation, WorldAimRotation);
	}

	if (FollowCamera)
	{
		FollowCamera->SetFieldOfView(CurrentFOV);
		CameraRuntimeState.SolvedArmLength = FVector::Distance(PivotWorldLocation, FollowCamera->GetComponentLocation());
	}
	else
	{
		CameraRuntimeState.SolvedArmLength = CurrentArmLength;
	}
}

// [v0.1.0] 현재 카메라 기준 Aim Trace를 계산합니다.
void UCFVehicleCameraComp::UpdateAimTrace(const FCFVehicleCameraTuningConfig& CameraTuningConfig)
{
	const AActor* OwnerActor = GetOwner();
	UWorld* CurrentWorld = GetWorld();
	if (!OwnerActor || !CurrentWorld)
	{
		CameraRuntimeState.AimHitLocation = FVector::ZeroVector;
		CameraRuntimeState.AimTraceDistance = 0.0f;
		CameraRuntimeState.bAimBlocked = false;
		CameraRuntimeState.bWeaponCanFireAtCurrentAim = true;
		return;
	}

	const FVector TraceStart = FollowCamera ? FollowCamera->GetComponentLocation() : GetPivotWorldLocation(VehicleCameraData ? VehicleCameraData->CameraTuningConfig : FCFVehicleCameraTuningConfig());
	const FVector TraceDirection = GetCurrentAimDirection().GetSafeNormal();
	const FVector TraceEnd = TraceStart + (TraceDirection * CameraTuningConfig.AimTraceLength);

	FHitResult AimHitResult;
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(CFVehicleCameraAimTrace), false, OwnerActor);
	const bool bHasBlockingHit = CurrentWorld->LineTraceSingleByChannel(AimHitResult, TraceStart, TraceEnd, ECC_Visibility, CollisionQueryParams);

	CameraRuntimeState.bAimBlocked = bHasBlockingHit;
	CameraRuntimeState.AimHitLocation = bHasBlockingHit ? AimHitResult.ImpactPoint : TraceEnd;
	CameraRuntimeState.AimTraceDistance = bHasBlockingHit ? AimHitResult.Distance : CameraTuningConfig.AimTraceLength;
	CameraRuntimeState.bWeaponCanFireAtCurrentAim = !CameraRuntimeState.bAimBlocked;

	if (bDrawAimTraceDebug)
	{
		const FColor DebugLineColor = bHasBlockingHit ? FColor::Red : FColor::Green;
		DrawDebugLine(CurrentWorld, TraceStart, CameraRuntimeState.AimHitLocation, DebugLineColor, false, 0.0f, 0, 1.5f);
		DrawDebugSphere(CurrentWorld, CameraRuntimeState.AimHitLocation, 8.0f, 8, DebugLineColor, false, 0.0f);
	}
}
