// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.1.6
// Date: 2026-07-20
// Description: CarFight 차량 카메라 컴포넌트 구현 초안 (WeaponHit Aim Trace 적용)
// Changelog:
// - v0.1.6: Camera Aim Trace가 선택한 Actor를 런타임 상태에 보존해 같은 목표 표면 판정을 지원.
// - v0.1.5: Camera Aim Trace를 WeaponHit 채널로 변경하고 표면 Hit을 AimBlocked와 분리.
// Migration:
// - AimTraceHitActor는 매 Aim Trace 결과로 갱신되며 미적중 또는 런타임 무효 시 비운다.
// - Camera Trace Hit은 Reticle 목표 표면 선택 결과로만 사용하고, 실제 발사 차단은 MuzzleBlocked 판정에서 처리한다.
// Scope: 차량 중심 피벗 기반 자유 조준, 제한각 Clamp, SpringArm 연동, Aim Trace 계산 골격을 구현합니다.

#include "CFVehicleCameraComp.h"

#include "CFCollisionChannels.h"
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

	if (ShouldSkipCameraRuntimeOnDedicatedServer())
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (bAutoInitializeOnBeginPlay)
	{
		InitializeCameraRuntime();
	}
}

// [v0.1.0] Tick에서 차량 상태를 읽어 카메라를 갱신합니다.
void UCFVehicleCameraComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShouldSkipCameraRuntimeOnDedicatedServer())
	{
		SetComponentTickEnabled(false);
		return;
	}

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
	if (ShouldSkipCameraRuntimeOnDedicatedServer())
	{
		bCameraRuntimeReady = false;
		return false;
	}

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
	CameraRuntimeState.SolvedArmLength = CurrentArmLength;

	// [v0.1.4] 초기 카메라 Yaw 완충 기준으로 사용할 차량 기본 Aim 회전입니다.
	const FRotator InitialBaseVehicleAimRotation = GetBaseVehicleAimRotation();
	SmoothedCameraBaseYawDeg = InitialBaseVehicleAimRotation.Yaw;
	bHasSmoothedCameraBaseYaw = true;
	CameraRuntimeState.bCameraYawDampingApplied = false;
	CameraRuntimeState.TargetCameraBaseYaw = InitialBaseVehicleAimRotation.Yaw;
	CameraRuntimeState.SmoothedCameraBaseYaw = InitialBaseVehicleAimRotation.Yaw;
	CameraRuntimeState.CameraYawDampingLagDeg = 0.0f;

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

// [v0.1.2] Dedicated Server에서 카메라 런타임 작업을 스킵해야 하는지 반환합니다.
bool UCFVehicleCameraComp::ShouldSkipCameraRuntimeOnDedicatedServer() const
{
	// [v0.1.2] 네트워크 모드를 확인할 현재 월드입니다.
	const UWorld* CurrentWorld = GetWorld();
	return CurrentWorld && CurrentWorld->GetNetMode() == NM_DedicatedServer;
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

// [v0.1.4] 카메라 표시용 기본 Aim Rotation에 Yaw 완충을 적용합니다.
FRotator UCFVehicleCameraComp::ResolveCameraYawDampedBaseRotation(float DeltaTime, const FRotator& TargetBaseVehicleAimRotation)
{
	// [v0.1.4] 카메라가 최종적으로 따라가야 하는 차량 기준 Yaw입니다.
	const float TargetCameraBaseYawDeg = TargetBaseVehicleAimRotation.Yaw;

	CameraRuntimeState.TargetCameraBaseYaw = TargetCameraBaseYawDeg;
	CameraRuntimeState.SmoothedCameraBaseYaw = TargetCameraBaseYawDeg;
	CameraRuntimeState.CameraYawDampingLagDeg = 0.0f;
	CameraRuntimeState.bCameraYawDampingApplied = false;

	if (!bEnableCameraYawDamping || CameraYawDampingInterpSpeed <= KINDA_SMALL_NUMBER || DeltaTime <= KINDA_SMALL_NUMBER)
	{
		SmoothedCameraBaseYawDeg = TargetCameraBaseYawDeg;
		bHasSmoothedCameraBaseYaw = false;
		return TargetBaseVehicleAimRotation;
	}

	if (!bHasSmoothedCameraBaseYaw)
	{
		SmoothedCameraBaseYawDeg = TargetCameraBaseYawDeg;
		bHasSmoothedCameraBaseYaw = true;
	}

	// [v0.1.4] 보간 전 현재 카메라 표시용 기본 회전입니다.
	const FRotator CurrentSmoothedBaseRotation(0.0f, SmoothedCameraBaseYawDeg, 0.0f);

	// [v0.1.4] 보간 목표로 사용할 차량 기준 기본 회전입니다.
	const FRotator TargetBaseRotation(0.0f, TargetCameraBaseYawDeg, 0.0f);

	// [v0.1.4] 이번 프레임 보간된 카메라 표시용 기본 회전입니다.
	const FRotator InterpedBaseRotation = FMath::RInterpTo(
		CurrentSmoothedBaseRotation,
		TargetBaseRotation,
		DeltaTime,
		CameraYawDampingInterpSpeed);

	SmoothedCameraBaseYawDeg = InterpedBaseRotation.Yaw;

	// [v0.1.4] 허용 가능한 최대 카메라 Yaw 지연각입니다.
	const float SafeMaxYawLagDeg = FMath::Max(0.0f, CameraYawDampingMaxLagDeg);

	// [v0.1.4] 차량 기준 Yaw 대비 현재 카메라 표시용 Yaw 지연각입니다.
	float CurrentYawLagDeg = FMath::FindDeltaAngleDegrees(TargetCameraBaseYawDeg, SmoothedCameraBaseYawDeg);
	if (SafeMaxYawLagDeg <= KINDA_SMALL_NUMBER)
	{
		SmoothedCameraBaseYawDeg = TargetCameraBaseYawDeg;
		CurrentYawLagDeg = 0.0f;
	}
	else if (FMath::Abs(CurrentYawLagDeg) > SafeMaxYawLagDeg)
	{
		CurrentYawLagDeg = FMath::Clamp(CurrentYawLagDeg, -SafeMaxYawLagDeg, SafeMaxYawLagDeg);
		SmoothedCameraBaseYawDeg = FRotator::NormalizeAxis(TargetCameraBaseYawDeg + CurrentYawLagDeg);
	}

	CameraRuntimeState.SmoothedCameraBaseYaw = SmoothedCameraBaseYawDeg;
	CameraRuntimeState.CameraYawDampingLagDeg = CurrentYawLagDeg;
	CameraRuntimeState.bCameraYawDampingApplied = true;

	return FRotator(TargetBaseVehicleAimRotation.Pitch, SmoothedCameraBaseYawDeg, 0.0f);
}

// [v0.1.3] 카메라 피벗의 월드 위치를 계산합니다.
FVector UCFVehicleCameraComp::GetPivotWorldLocation(const FCFVehicleCameraTuningConfig& CameraTuningConfig) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FVector::ZeroVector;
	}

	// 차량 Root 기준 위치를 얻기 위한 Owner Root Component입니다.
	const USceneComponent* OwnerRootComponent = OwnerActor->GetRootComponent();

	// 차량 Root가 없을 때 사용할 Actor 기준 위치입니다.
	const FVector OwnerBaseLocation = OwnerRootComponent ? OwnerRootComponent->GetComponentLocation() : OwnerActor->GetActorLocation();

	// 차량 Pitch/Roll을 제거한 카메라 기준 Yaw 회전입니다.
	const FRotator BaseVehicleAimRotation = GetBaseVehicleAimRotation();

	// 카메라 피벗 계산의 기준 월드 위치입니다.
	FVector PivotBaseLocation = OwnerBaseLocation;

	if (CameraPivotRoot)
	{
		if (bIsolateCameraFromVehiclePitchRoll)
		{
			// Root 기준 카메라 Pivot의 로컬 위치입니다.
			const FVector PivotOwnerLocalLocation = OwnerRootComponent
				? OwnerRootComponent->GetComponentTransform().InverseTransformPosition(CameraPivotRoot->GetComponentLocation())
				: CameraPivotRoot->GetRelativeLocation();

			PivotBaseLocation = OwnerBaseLocation + BaseVehicleAimRotation.RotateVector(PivotOwnerLocalLocation);
		}
		else
		{
			PivotBaseLocation = CameraPivotRoot->GetComponentLocation();
		}
	}

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

// [v0.1.1] 목표 카메라 위치와 FOV를 계산해 SpringArm / Camera에 반영합니다.
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

	// 이전 프레임 실제 해결 거리 기준으로 충돌 압축 비율을 계산합니다.
	const float PreviousSolvedArmLength = CameraRuntimeState.SolvedArmLength > KINDA_SMALL_NUMBER
		? CameraRuntimeState.SolvedArmLength
		: CurrentArmLength;
	const float CollisionCompressionRatio = DesiredArmLength > KINDA_SMALL_NUMBER
		? FMath::Clamp(PreviousSolvedArmLength / DesiredArmLength, 0.0f, 1.0f)
		: 1.0f;

	// 충돌로 카메라가 크게 당겨졌다면 높이와 FOV를 조금 보조해 가시성을 유지합니다.
	if (CameraTuningConfig.bUseCollisionViewAssist && CollisionCompressionRatio < CameraTuningConfig.CollisionViewAssistStartRatio)
	{
		const float CollisionAssistAlpha = 1.0f - FMath::Clamp(
			CollisionCompressionRatio / CameraTuningConfig.CollisionViewAssistStartRatio,
			0.0f,
			1.0f);
		ResolvedHeightOffset += CameraTuningConfig.MaxCollisionHeightAssist * CollisionAssistAlpha;
		DesiredFOV += CameraTuningConfig.MaxCollisionFOVAssist * CollisionAssistAlpha;
	}

	CurrentArmLength = FMath::FInterpTo(CurrentArmLength, DesiredArmLength, DeltaTime, CameraTuningConfig.ArmLengthInterpSpeed);
	CurrentFOV = FMath::FInterpTo(CurrentFOV, DesiredFOV, DeltaTime, CameraTuningConfig.FOVInterpSpeed);

	CameraRuntimeState.DesiredArmLength = DesiredArmLength;
	CameraRuntimeState.CurrentArmLength = CurrentArmLength;
	CameraRuntimeState.DesiredFOV = DesiredFOV;
	CameraRuntimeState.CurrentFOV = CurrentFOV;
	CameraRuntimeState.bCameraPitchRollIsolationApplied = bIsolateCameraFromVehiclePitchRoll;

	const FVector PivotWorldLocation = GetPivotWorldLocation(CameraTuningConfig);
	const FRotator BaseVehicleAimRotation = GetBaseVehicleAimRotation();

	// [v0.1.4] 화면 표시용으로 Yaw 완충이 적용된 카메라 기준 Aim 회전입니다.
	const FRotator CameraBaseAimRotation = ResolveCameraYawDampedBaseRotation(DeltaTime, BaseVehicleAimRotation);
	const FRotator WorldAimRotation(
		CameraBaseAimRotation.Pitch + CameraRuntimeState.ClampedAimPitch,
		CameraBaseAimRotation.Yaw + CameraRuntimeState.ClampedAimYaw,
		0.0f);

	if (CameraAimPivot)
	{
		if (bIsolateCameraFromVehiclePitchRoll)
		{
			CameraAimPivot->SetUsingAbsoluteRotation(true);
		}

		CameraAimPivot->SetWorldLocationAndRotation(PivotWorldLocation, WorldAimRotation);
	}

	if (CameraBoom)
	{
		CameraBoom->bUsePawnControlRotation = false;
		if (bIsolateCameraFromVehiclePitchRoll)
		{
			CameraBoom->SetUsingAbsoluteRotation(true);
			CameraBoom->bInheritPitch = false;
			CameraBoom->bInheritYaw = false;
			CameraBoom->bInheritRoll = false;
		}

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

	// SpringArm 충돌로 실제 거리가 더 짧아졌다면 내부 현재 Arm 길이도 함께 낮춰
	// 충돌 해제 후 즉시 튀지 않고 코드 보간으로 천천히 복귀하도록 맞춥니다.
	if (CameraBoom && CameraTuningConfig.bEnableBoomCollisionTest)
	{
		const float CollisionSolvedArmLength = FMath::Max(CameraTuningConfig.MinArmLength, CameraRuntimeState.SolvedArmLength);
		if (CollisionSolvedArmLength + KINDA_SMALL_NUMBER < CurrentArmLength)
		{
			CurrentArmLength = CollisionSolvedArmLength;
			CameraRuntimeState.CurrentArmLength = CurrentArmLength;
		}
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
		CameraRuntimeState.bAimTraceHasBlockingHit = false;
		CameraRuntimeState.AimTraceHitActor = nullptr;
		CameraRuntimeState.bWeaponCanFireAtCurrentAim = true;
		return;
	}

	const FVector TraceStart = FollowCamera ? FollowCamera->GetComponentLocation() : GetPivotWorldLocation(VehicleCameraData ? VehicleCameraData->CameraTuningConfig : FCFVehicleCameraTuningConfig());
	const FVector TraceDirection = GetCurrentAimDirection().GetSafeNormal();
	const FVector TraceEnd = TraceStart + (TraceDirection * CameraTuningConfig.AimTraceLength);

	FHitResult AimHitResult;
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(CFVehicleCameraAimTrace), false, OwnerActor);
	const bool bHasBlockingHit = CurrentWorld->LineTraceSingleByChannel(AimHitResult, TraceStart, TraceEnd, CFCollisionChannels::WeaponHit, CollisionQueryParams);

	CameraRuntimeState.bAimBlocked = false;
	CameraRuntimeState.bAimTraceHasBlockingHit = bHasBlockingHit;
	CameraRuntimeState.AimTraceHitActor = bHasBlockingHit ? AimHitResult.GetActor() : nullptr;
	CameraRuntimeState.AimHitLocation = bHasBlockingHit ? AimHitResult.ImpactPoint : TraceEnd;
	CameraRuntimeState.AimTraceDistance = bHasBlockingHit ? AimHitResult.Distance : CameraTuningConfig.AimTraceLength;
	CameraRuntimeState.bWeaponCanFireAtCurrentAim = true;

	if (bDrawAimTraceDebug)
	{
		const FColor DebugLineColor = bHasBlockingHit ? FColor::Red : FColor::Green;
		DrawDebugLine(CurrentWorld, TraceStart, CameraRuntimeState.AimHitLocation, DebugLineColor, false, 0.0f, 0, 1.5f);
		DrawDebugSphere(CurrentWorld, CameraRuntimeState.AimHitLocation, 8.0f, 8, DebugLineColor, false, 0.0f);
	}
}
