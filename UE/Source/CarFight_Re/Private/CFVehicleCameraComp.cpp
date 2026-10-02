// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.4.1
// Date: 2026-10-02
// Description: VCFX-P0-03 SpringArm collision boundary chatter를 Clear hold/hysteresis 기반 Recovery로 교정
// Changelog:
// - v0.4.1: collision solved distance를 CurrentArmLength에 역주입하는 feedback loop를 제거하고 full desired path Sweep + Clear hold + padded hysteresis + smooth recovery를 추가.
// - v0.4.0: SpeedRatio를 CameraData.MaxSpeedPresentationRatio까지 허용해 ReferenceMaxSpeedKmh 초과에서도 Speed FOV/Arm이 계속 진행하도록 교정.
// - v0.3.0: Lateral Roll을 횡가속 Curve × 실제 차체 Motion intensity × 자유시점 signed ViewAlignment × 기존 Motion 감쇠로 합성하도록 교정.
// - v0.2.2: normalized Speed/Acceleration/Braking/Lateral 수식을 Private pure helper로 이동해 런타임과 focused Automation이 동일 계산 authority를 공유하도록 보강.
// - v0.2.1: invalid Reference/Velocity/Axis에서는 Curve의 0-input authored 값과 무관하게 normalized Driving FX 전체를 exact0으로 fail-safe하는 input-valid gate 추가.
// - v0.2.0: 차량별 Presentation CameraData resolve, normalized planar motion/fail-safe, Speed FOV/Arm, Accel/Brake Kick, Lateral Roll, Gameplay View를 additive 구현.
// - v0.1.6: Camera Aim Trace가 선택한 Actor를 런타임 상태에 보존해 같은 목표 표면 판정을 지원.
// - v0.1.5: Camera Aim Trace를 WeaponHit 채널로 변경하고 표면 Hit을 AimBlocked와 분리.
// Migration:
// - AimTraceHitActor는 매 Aim Trace 결과로 갱신되며 미적중 또는 런타임 무효 시 비운다.
// - Camera Trace Hit은 Reticle 목표 표면 선택 결과로만 사용하고, 실제 발사 차단은 MuzzleBlocked 판정에서 처리한다.
// Scope: 차량 중심 피벗 기반 자유 조준, 제한각 Clamp, SpringArm 연동, Aim Trace 계산 골격을 구현합니다.

#include "CFVehicleCameraComp.h"

#include "CFVehicleCameraMath.h"
#include "CFCollisionChannels.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"

namespace
{
	// VCFX-P0-02 normalized motion에서 허용하는 최소 DeltaTime입니다.
	constexpr float MinValidDrivingDeltaTimeSec = 0.0001f;

	// VCFX-P0-02 normalized motion에서 허용하는 최대 DeltaTime입니다.
	constexpr float MaxValidDrivingDeltaTimeSec = 0.10f;

	// Teleport 판정의 최소 XY 거리입니다.
	constexpr float MinTeleportResetDistanceCm = 2000.0f;

	// 기준속도 기반 Teleport 허용거리에 적용할 배율입니다.
	constexpr float TeleportDistanceMultiplier = 3.0f;

	// Vector의 모든 성분이 finite인지 확인합니다.
	bool IsFiniteCameraVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y) && FMath::IsFinite(Value.Z);
	}

	// RuntimeFloatCurve를 안전하게 평가하고 non-finite 출력은 0으로 fail-safe 합니다.
	float EvaluateFiniteCurve(const FRuntimeFloatCurve& Curve, const float InputValue)
	{
		const FRichCurve* RichCurve = Curve.GetRichCurveConst();
		const float EvaluatedValue = RichCurve ? RichCurve->Eval(InputValue, 0.0f) : 0.0f;
		return FMath::IsFinite(EvaluatedValue) ? EvaluatedValue : 0.0f;
	}

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

	// 차량별 Presentation override를 포함해 이번 frame에 사용할 Camera tuning source를 해석합니다.
	const UCFVehicleCameraData* ResolvedPresentationData = ResolveCameraPresentationData();
	const FCFVehicleCameraTuningConfig LocalCameraTuningConfig = ResolvedPresentationData ? ResolvedPresentationData->CameraTuningConfig : FCFVehicleCameraTuningConfig();
	// Aim gameplay는 per-vehicle Presentation override와 분리된 기존 owner 계약을 유지합니다.
	const FCFVehicleCameraAimProfile LocalAimProfile = BuildResolvedAimProfile();

	CameraRuntimeState.PreviousCameraMode = CameraRuntimeState.CurrentCameraMode;
	CameraRuntimeState.CurrentCameraMode = EvaluateCameraMode();
	CameraRuntimeState.bCameraModeChangedThisFrame = CameraRuntimeState.PreviousCameraMode != CameraRuntimeState.CurrentCameraMode;
	CameraRuntimeState.ActiveAimProfileName = LocalAimProfile.ProfileName;

	UpdateAimState(DeltaTime, LocalCameraTuningConfig, LocalAimProfile);
	UpdateDrivingMotionState(DeltaTime, LocalCameraTuningConfig);
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

	// 초기화 시점에도 VehicleData Presentation override를 같은 resolver로 해석합니다.
	const UCFVehicleCameraData* ResolvedPresentationData = ResolveCameraPresentationData();
	const FCFVehicleCameraTuningConfig LocalCameraTuningConfig = ResolvedPresentationData ? ResolvedPresentationData->CameraTuningConfig : FCFVehicleCameraTuningConfig();
	CurrentArmLength = LocalCameraTuningConfig.BaseArmLength;
	// 충돌 복귀 상태는 Presentation 기본 Arm에서 시작합니다.
	CollisionRecoveryArmLength = CurrentArmLength;
	// 초기화 시 연속 Clear 누적 시간을 비웁니다.
	CollisionClearElapsedSec = 0.0f;
	// 초기화 시 충돌 복귀 상태를 비활성화합니다.
	bCollisionRecoveryActive = false;
	CurrentFOV = LocalCameraTuningConfig.BaseFOV;
	CurrentGameplayFOV = CurrentFOV;
	CameraRuntimeState.CurrentArmLength = CurrentArmLength;
	CameraRuntimeState.CurrentFOV = CurrentFOV;
	CameraRuntimeState.CurrentGameplayFOV = CurrentGameplayFOV;
	CameraRuntimeState.SolvedArmLength = CurrentArmLength;

	// [v0.1.4] 초기 카메라 Yaw 완충 기준으로 사용할 차량 기본 Aim 회전입니다.
	const FRotator InitialBaseVehicleAimRotation = GetBaseVehicleAimRotation();
	SmoothedCameraBaseYawDeg = InitialBaseVehicleAimRotation.Yaw;
	bHasSmoothedCameraBaseYaw = true;
	CameraRuntimeState.bCameraYawDampingApplied = false;
	CameraRuntimeState.TargetCameraBaseYaw = InitialBaseVehicleAimRotation.Yaw;
	CameraRuntimeState.SmoothedCameraBaseYaw = InitialBaseVehicleAimRotation.Yaw;
	CameraRuntimeState.CameraYawDampingLagDeg = 0.0f;

	// Driving Roll이 누적되지 않도록 초기 FollowCamera 상대 회전을 baseline으로 보존합니다.
	if (FollowCamera)
	{
		BaseFollowCameraRelativeRotation = FollowCamera->GetRelativeRotation();
		bHasBaseFollowCameraRelativeRotation = true;
	}
	else
	{
		BaseFollowCameraRelativeRotation = FRotator::ZeroRotator;
		bHasBaseFollowCameraRelativeRotation = false;
	}
	bPresentationRollAppliedLastFrame = false;
	bHasValidDrivingMotionHistory = false;
	FilteredAccelerationRate = 0.0f;
	FilteredBrakingRate = 0.0f;
	FilteredLateralRate = 0.0f;
	CurrentGameplayView = FCFVehicleCameraGameplayView();

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

// [v0.2.0] Presentation Roll/Speed FOV에 오염되지 않은 Gameplay View를 반환합니다.
FCFVehicleCameraGameplayView UCFVehicleCameraComp::GetGameplayView() const
{
	return CurrentGameplayView;
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
	if (CurrentGameplayView.bValid && !CurrentGameplayView.ViewDirection.IsNearlyZero())
	{
		return CurrentGameplayView.ViewDirection;
	}

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

FVector UCFVehicleCameraComp::GetCurrentAimTraceStartLocation() const
{
	if (CurrentGameplayView.bValid)
	{
		return CurrentGameplayView.ViewOrigin;
	}

	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}

	if (CameraAimPivot)
	{
		return CameraAimPivot->GetComponentLocation();
	}

	if (CameraPivotRoot)
	{
		return CameraPivotRoot->GetComponentLocation();
	}

	const AActor* OwnerActor = GetOwner();
	return OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
}

// 현재 Aim Trace 적중 위치를 반환합니다.
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

// [v0.2.0] VehicleData Presentation override → Component fallback 순서로 CameraData를 해석합니다.
const UCFVehicleCameraData* UCFVehicleCameraComp::ResolveCameraPresentationData() const
{
	// 차량별 Presentation override를 조회할 소유 Vehicle Pawn입니다.
	const ACFVehiclePawn* OwnerVehiclePawn = CachedVehiclePawn.IsValid() ? CachedVehiclePawn.Get() : Cast<ACFVehiclePawn>(GetOwner());
	if (OwnerVehiclePawn && OwnerVehiclePawn->VehicleData && OwnerVehiclePawn->VehicleData->CameraPresentationDataOverride)
	{
		return OwnerVehiclePawn->VehicleData->CameraPresentationDataOverride;
	}

	return VehicleCameraData;
}

// [v0.3.0] 차량의 평면 운동과 실제 차체 Motion을 normalized Driving FX 입력으로 갱신합니다.
void UCFVehicleCameraComp::UpdateDrivingMotionState(float DeltaTime, const FCFVehicleCameraTuningConfig& CameraTuningConfig)
{
	// 이번 frame의 normalized Driving FX 계산에 사용할 설정입니다.
	const FCFVehicleDrivingFXConfig& DrivingFXConfig = CameraTuningConfig.DrivingFXConfig;

	CameraRuntimeState.bNormalizedDrivingFXActive = DrivingFXConfig.bUseNormalizedDrivingFX;
	CameraRuntimeState.bMotionHistoryResetThisFrame = false;
	bDrivingMotionInputValid = false;
	CameraRuntimeState.SpeedRatio = 0.0f;
	CameraRuntimeState.AccelerationRate = 0.0f;
	CameraRuntimeState.BrakingRate = 0.0f;
	CameraRuntimeState.LateralRate = 0.0f;
	CameraRuntimeState.VehicleBodyRollDeg = 0.0f;
	CameraRuntimeState.VehicleBodyRollRateDegPerSec = 0.0f;
	CameraRuntimeState.VehicleBodyYawRateDegPerSec = 0.0f;
	CameraRuntimeState.LateralBodyMotionIntensity = 0.0f;

	if (!DrivingFXConfig.bUseNormalizedDrivingFX)
	{
		bHasValidDrivingMotionHistory = false;
		FilteredAccelerationRate = 0.0f;
		FilteredBrakingRate = 0.0f;
		FilteredLateralRate = 0.0f;
		FilteredLateralBodyMotionIntensity = 0.0f;
		return;
	}

	// normalized motion source를 제공할 실제 Vehicle Pawn입니다.
	ACFVehiclePawn* OwnerVehiclePawn = ResolveVehiclePawnOwner();

	// 차량별 Camera performance reference를 소유한 VehicleData입니다.
	const UCFVehicleData* VehicleData = OwnerVehiclePawn ? OwnerVehiclePawn->VehicleData.Get() : nullptr;

	// 차량별 authored 기준 최고속도(km/h)입니다.
	const float ReferenceMaxSpeedKmh = VehicleData ? VehicleData->ReferenceMaxSpeedKmh : 0.0f;

	if (!OwnerVehiclePawn || !FMath::IsFinite(ReferenceMaxSpeedKmh) || ReferenceMaxSpeedKmh <= KINDA_SMALL_NUMBER)
	{
		bHasValidDrivingMotionHistory = false;
		FilteredAccelerationRate = 0.0f;
		FilteredBrakingRate = 0.0f;
		FilteredLateralRate = 0.0f;
		FilteredLateralBodyMotionIntensity = 0.0f;
		return;
	}

	// ReferenceMaxSpeedKmh를 Unreal 단위(cm/s)로 환산한 공통 정규화 분모입니다.
	const float ReferenceSpeedCmPerSec = ReferenceMaxSpeedKmh / 0.036f;

	// 수직 성분을 제거한 현재 차량 평면 속도입니다.
	FVector CurrentPlanarVelocity = OwnerVehiclePawn->GetVelocity();
	CurrentPlanarVelocity.Z = 0.0f;

	// Teleport/hitch 판정을 위한 현재 차량 평면 위치입니다.
	FVector CurrentPlanarLocation = OwnerVehiclePawn->GetActorLocation();
	CurrentPlanarLocation.Z = 0.0f;

	// 종방향 속도 변화를 계산할 차량 Forward 평면 축입니다.
	FVector VehicleForwardXY = OwnerVehiclePawn->GetActorForwardVector();
	VehicleForwardXY.Z = 0.0f;

	// 횡가속 부호와 크기를 계산할 차량 Right 평면 축입니다.
	FVector VehicleRightXY = OwnerVehiclePawn->GetActorRightVector();
	VehicleRightXY.Z = 0.0f;

	if (!FMath::IsFinite(ReferenceSpeedCmPerSec)
		|| ReferenceSpeedCmPerSec <= KINDA_SMALL_NUMBER
		|| !IsFiniteCameraVector(CurrentPlanarVelocity)
		|| !IsFiniteCameraVector(CurrentPlanarLocation)
		|| !IsFiniteCameraVector(VehicleForwardXY)
		|| !IsFiniteCameraVector(VehicleRightXY)
		|| !VehicleForwardXY.Normalize()
		|| !VehicleRightXY.Normalize())
	{
		bHasValidDrivingMotionHistory = false;
		FilteredAccelerationRate = 0.0f;
		FilteredBrakingRate = 0.0f;
		FilteredLateralRate = 0.0f;
		FilteredLateralBodyMotionIntensity = 0.0f;
		return;
	}

	// Reference/Velocity/Axis가 모두 valid인 frame만 normalized Curve 평가를 허용합니다.
	bDrivingMotionInputValid = true;

	// 현재 XY 속력의 절댓값입니다.
	const float CurrentPlanarSpeed = CurrentPlanarVelocity.Size();
	CameraRuntimeState.SpeedRatio = CFVehicleCameraMath::NormalizeSpeedRatio(
		CurrentPlanarSpeed,
		ReferenceSpeedCmPerSec,
		DrivingFXConfig.MaxSpeedPresentationRatio);

	// 현재 차량 진행축 기준 속도의 절댓값입니다.
	const float CurrentForwardSpeedAbs = FMath::Abs(FVector::DotProduct(CurrentPlanarVelocity, VehicleForwardXY));

	// 차체 Motion 보간에 사용할 이번 frame DeltaTime이 유효한지 여부입니다.
	const bool bValidDrivingDeltaTime = FMath::IsFinite(DeltaTime)
		&& DeltaTime >= MinValidDrivingDeltaTimeSec
		&& DeltaTime <= MaxValidDrivingDeltaTimeSec;

	if (bValidDrivingDeltaTime)
	{
		// Chaos Wheeled Vehicle의 실제 물리 차체 SkeletalMesh입니다.
		USkeletalMeshComponent* VehicleMeshComponent = OwnerVehiclePawn->GetMesh();

		// 실제 차체 Motion sample이 이번 frame에 유효한지 여부입니다.
		bool bValidBodyMotionSample = false;

		// 실제 차체 Roll 각도입니다.
		float VehicleBodyRollDeg = 0.0f;

		// 실제 차체 local X축 기준 Roll 각속도입니다.
		float VehicleBodyRollRateDegPerSec = 0.0f;

		// 실제 차체 local Z축 기준 Yaw 각속도입니다.
		float VehicleBodyYawRateDegPerSec = 0.0f;

		if (VehicleMeshComponent)
		{
			// 실제 차체 Component 월드 회전입니다.
			const FRotator VehicleBodyRotation = VehicleMeshComponent->GetComponentRotation();

			// 실제 차체 월드 각속도(deg/s)입니다.
			const FVector WorldAngularVelocityDegPerSec = VehicleMeshComponent->GetPhysicsAngularVelocityInDegrees();

			// 차체 로컬 축 기준 각속도로 변환한 값입니다.
			const FVector LocalAngularVelocityDegPerSec =
				VehicleMeshComponent->GetComponentTransform().InverseTransformVectorNoScale(WorldAngularVelocityDegPerSec);

			bValidBodyMotionSample =
				FMath::IsFinite(VehicleBodyRotation.Roll)
				&& IsFiniteCameraVector(WorldAngularVelocityDegPerSec)
				&& IsFiniteCameraVector(LocalAngularVelocityDegPerSec);

			if (bValidBodyMotionSample)
			{
				VehicleBodyRollDeg = FRotator::NormalizeAxis(VehicleBodyRotation.Roll);
				VehicleBodyRollRateDegPerSec = LocalAngularVelocityDegPerSec.X;
				VehicleBodyYawRateDegPerSec = LocalAngularVelocityDegPerSec.Z;
			}
		}

		if (bValidBodyMotionSample)
		{
			// Roll angle / Roll rate / Yaw rate 중 가장 강한 0~1 차체 Motion 목표 강도입니다.
			const float LateralBodyMotionIntensityTarget =
				CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
					VehicleBodyRollDeg,
					VehicleBodyRollRateDegPerSec,
					VehicleBodyYawRateDegPerSec,
					DrivingFXConfig.LateralBodyRollAngleForFullIntensityDeg,
					DrivingFXConfig.LateralBodyRollRateForFullIntensityDegPerSec,
					DrivingFXConfig.LateralBodyYawRateForFullIntensityDegPerSec);

			FilteredLateralBodyMotionIntensity = FMath::FInterpTo(
				FilteredLateralBodyMotionIntensity,
				LateralBodyMotionIntensityTarget,
				DeltaTime,
				DrivingFXConfig.LateralBodyMotionInterpSpeed);

			if (!FMath::IsFinite(FilteredLateralBodyMotionIntensity))
			{
				FilteredLateralBodyMotionIntensity = 0.0f;
			}

			CameraRuntimeState.VehicleBodyRollDeg = VehicleBodyRollDeg;
			CameraRuntimeState.VehicleBodyRollRateDegPerSec = VehicleBodyRollRateDegPerSec;
			CameraRuntimeState.VehicleBodyYawRateDegPerSec = VehicleBodyYawRateDegPerSec;
			CameraRuntimeState.LateralBodyMotionIntensity =
				FMath::Clamp(FilteredLateralBodyMotionIntensity, 0.0f, 1.0f);
		}
		else
		{
			// 실제 차체 Motion 증거가 없으면 카메라 Roll 강도를 exact0으로 fail-safe합니다.
			FilteredLateralBodyMotionIntensity = 0.0f;
		}
	}
	else
	{
		// 비정상 DeltaTime에서는 차체 Motion 보간도 사용하지 않습니다.
		FilteredLateralBodyMotionIntensity = 0.0f;
	}

	// 차분 계산을 중단하고 현재 frame을 새 history 기준점으로 재설정하는 로컬 함수입니다.
	auto ResetMotionHistory = [&]()
	{
		PreviousPlanarVelocity = CurrentPlanarVelocity;
		PreviousPlanarLocation = CurrentPlanarLocation;
		PreviousForwardSpeedAbs = CurrentForwardSpeedAbs;
		bHasValidDrivingMotionHistory = true;
		FilteredAccelerationRate = 0.0f;
		FilteredBrakingRate = 0.0f;
		FilteredLateralRate = 0.0f;
		CameraRuntimeState.bMotionHistoryResetThisFrame = true;
	};

	if (!bValidDrivingDeltaTime || !bHasValidDrivingMotionHistory)
	{
		ResetMotionHistory();
		return;
	}

	// 정상 주행으로 허용할 최대 frame 간 평면 이동 거리입니다.
	const float AllowedPlanarDistance = FMath::Max(
		MinTeleportResetDistanceCm,
		ReferenceSpeedCmPerSec * DeltaTime * TeleportDistanceMultiplier);

	// 실제 frame 간 평면 이동 거리입니다.
	const float ActualPlanarLocationDelta = FVector::Distance(CurrentPlanarLocation, PreviousPlanarLocation);

	if (!FMath::IsFinite(ActualPlanarLocationDelta) || ActualPlanarLocationDelta > AllowedPlanarDistance)
	{
		ResetMotionHistory();
		return;
	}

	// 이전 frame 대비 진행축 속도 절댓값 변화율입니다.
	const float LongitudinalSpeedRate = (CurrentForwardSpeedAbs - PreviousForwardSpeedAbs) / DeltaTime;

	// 이전 frame 대비 XY 속도 변화로 계산한 평면 가속도입니다.
	const FVector PlanarAcceleration = (CurrentPlanarVelocity - PreviousPlanarVelocity) / DeltaTime;

	// 차량 Right 축에 투영한 signed 횡가속도입니다.
	const float LateralAcceleration = FVector::DotProduct(PlanarAcceleration, VehicleRightXY);

	// 0~1로 정규화한 가속 목표율입니다.
	const float AccelerationRateTarget =
		CFVehicleCameraMath::NormalizeAccelerationRate(LongitudinalSpeedRate, ReferenceSpeedCmPerSec);

	// 0~1로 정규화한 제동 목표율입니다.
	const float BrakingRateTarget =
		CFVehicleCameraMath::NormalizeBrakingRate(LongitudinalSpeedRate, ReferenceSpeedCmPerSec);

	// -1~1로 정규화한 signed 횡가속 목표율입니다.
	const float LateralRateTarget =
		CFVehicleCameraMath::NormalizeLateralRate(LateralAcceleration, ReferenceSpeedCmPerSec);

	FilteredAccelerationRate = FMath::FInterpTo(
		FilteredAccelerationRate,
		AccelerationRateTarget,
		DeltaTime,
		DrivingFXConfig.MotionRateInterpSpeed);

	FilteredBrakingRate = FMath::FInterpTo(
		FilteredBrakingRate,
		BrakingRateTarget,
		DeltaTime,
		DrivingFXConfig.MotionRateInterpSpeed);

	FilteredLateralRate = FMath::FInterpTo(
		FilteredLateralRate,
		LateralRateTarget,
		DeltaTime,
		DrivingFXConfig.MotionRateInterpSpeed);

	if (!FMath::IsFinite(FilteredAccelerationRate)
		|| !FMath::IsFinite(FilteredBrakingRate)
		|| !FMath::IsFinite(FilteredLateralRate))
	{
		ResetMotionHistory();
		return;
	}

	CameraRuntimeState.AccelerationRate = FMath::Clamp(FilteredAccelerationRate, 0.0f, 1.0f);
	CameraRuntimeState.BrakingRate = FMath::Clamp(FilteredBrakingRate, 0.0f, 1.0f);
	CameraRuntimeState.LateralRate = FMath::Clamp(FilteredLateralRate, -1.0f, 1.0f);

	PreviousPlanarVelocity = CurrentPlanarVelocity;
	PreviousPlanarLocation = CurrentPlanarLocation;
	PreviousForwardSpeedAbs = CurrentForwardSpeedAbs;
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

// [v0.2.0] 목표 Gameplay/Presentation 카메라 상태를 계산해 SpringArm / Camera에 반영합니다.
void UCFVehicleCameraComp::UpdateCameraTransform(float DeltaTime, const FCFVehicleCameraTuningConfig& CameraTuningConfig, const FCFVehicleCameraAimProfile& AimProfile)
{
	// 이번 frame Presentation 계산에 사용할 Driving FX 설정입니다.
	const FCFVehicleDrivingFXConfig& DrivingFXConfig = CameraTuningConfig.DrivingFXConfig;
	// 신규 normalized 경로와 기존 legacy 경로를 선택하는 명시적 전환값입니다.
	const bool bUseNormalizedDrivingFX = DrivingFXConfig.bUseNormalizedDrivingFX;

	// 이전 frame에 적용한 Roll을 먼저 baseline으로 복구해 이번 Gameplay View가 이전 Presentation Roll을 상속하지 않게 합니다.
	if (FollowCamera && bPresentationRollAppliedLastFrame && bHasBaseFollowCameraRelativeRotation)
	{
		FollowCamera->SetRelativeRotation(BaseFollowCameraRelativeRotation);
		bPresentationRollAppliedLastFrame = false;
	}

	// 기존 mode/Aim offset까지만 포함할 Gameplay 의미의 목표 Arm 길이입니다.
	float GameplayDesiredArmLength = CameraTuningConfig.BaseArmLength;
	// 신규 Speed FOV를 제외한 Gameplay 의미의 목표 FOV입니다.
	float GameplayDesiredFOV = CameraTuningConfig.BaseFOV;
	// CameraData와 AimProfile을 합성한 최종 높이 오프셋입니다.
	float ResolvedHeightOffset = CameraTuningConfig.BaseHeightOffset + AimProfile.HeightOffset;
	// CameraData와 AimProfile을 합성한 최종 측면 오프셋입니다.
	const float ResolvedSideOffset = CameraTuningConfig.BaseSideOffset + AimProfile.SideOffset;

	if (CameraRuntimeState.CurrentCameraMode == ECFVehicleCameraMode::Combat)
	{
		GameplayDesiredArmLength += CameraTuningConfig.CombatArmLengthOffset;
		GameplayDesiredFOV += CameraTuningConfig.CombatFOVOffset;
	}
	else if (CameraRuntimeState.CurrentCameraMode == ECFVehicleCameraMode::Reverse)
	{
		GameplayDesiredArmLength += CameraTuningConfig.ReverseArmLengthOffset;
		GameplayDesiredFOV += CameraTuningConfig.ReverseFOVOffset;
	}
	else if (CameraRuntimeState.CurrentCameraMode == ECFVehicleCameraMode::Airborne)
	{
		GameplayDesiredFOV += CameraTuningConfig.AirborneFOVOffset;
	}

	GameplayDesiredArmLength += AimProfile.ArmLengthOffset;
	GameplayDesiredFOV += AimProfile.FOVOffset;

	// 신규 Driving FX까지 더할 최종 Presentation Arm 목표값입니다.
	float PresentationDesiredArmLength = GameplayDesiredArmLength;
	// 신규 Driving FX까지 더할 최종 Presentation FOV 목표값입니다.
	float PresentationDesiredFOV = GameplayDesiredFOV;
	// 신규 Driving FX가 적용할 최종 Presentation Roll 각도입니다.
	float PresentationRollDeg = 0.0f;

	CameraRuntimeState.ResolvedSpeedFXScale = 1.0f;
	CameraRuntimeState.ResolvedMotionFXScale = 1.0f;
	CameraRuntimeState.SpeedFOVOffsetDeg = 0.0f;
	CameraRuntimeState.SpeedArmOffsetCm = 0.0f;
	CameraRuntimeState.AccelerationArmKickCm = 0.0f;
	CameraRuntimeState.BrakingArmKickCm = 0.0f;
	CameraRuntimeState.LateralRollDeg = 0.0f;

	if (bUseNormalizedDrivingFX)
	{
		// 여러 상태 중 가장 강한 감쇠를 반영할 Speed FOV/Arm 최종 배율입니다.
		float ResolvedSpeedFXScale = 1.0f;
		// Accel/Brake/Roll 계열에 적용할 최종 상태 감쇠 배율입니다.
		float ResolvedMotionFXScale = 1.0f;
		// 동시에 활성인 여러 상태 중 가장 강한 감쇠를 선택하는 로컬 함수입니다.
		auto ApplyAttenuation = [&](const float SpeedScale, const float MotionScale)
		{
			ResolvedSpeedFXScale = FMath::Min(ResolvedSpeedFXScale, FMath::Clamp(SpeedScale, 0.0f, 1.0f));
			ResolvedMotionFXScale = FMath::Min(ResolvedMotionFXScale, FMath::Clamp(MotionScale, 0.0f, 1.0f));
		};

		if (CameraModeFlags.bCombat)
		{
			ApplyAttenuation(DrivingFXConfig.CombatSpeedFXScale, DrivingFXConfig.CombatMotionFXScale);
		}
		if (CameraModeFlags.bAimPresentation)
		{
			ApplyAttenuation(DrivingFXConfig.AimSpeedFXScale, DrivingFXConfig.AimMotionFXScale);
		}
		if (CameraModeFlags.bAirborne)
		{
			ApplyAttenuation(DrivingFXConfig.AirborneSpeedFXScale, DrivingFXConfig.AirborneMotionFXScale);
		}
		if (CameraModeFlags.bReverse)
		{
			ApplyAttenuation(DrivingFXConfig.ReverseSpeedFXScale, DrivingFXConfig.ReverseMotionFXScale);
		}
		if (CameraModeFlags.bDestroyed || CameraModeFlags.bSpectate)
		{
			ResolvedSpeedFXScale = 0.0f;
			ResolvedMotionFXScale = 0.0f;
		}

		CameraRuntimeState.ResolvedSpeedFXScale = ResolvedSpeedFXScale;
		CameraRuntimeState.ResolvedMotionFXScale = ResolvedMotionFXScale;

		// Reference/Input invalid이면 authored Curve의 0-input 값과 무관하게 모든 신규 Driving FX 출력을 exact0으로 유지합니다.
		// 기존 0~100% Speed Curve가 소비할 0~1 기본 속도 비율입니다.
		const float BaseSpeedCurveRatio = FMath::Clamp(CameraRuntimeState.SpeedRatio, 0.0f, 1.0f);

		// ReferenceMaxSpeedKmh 초과 구간의 0~1 Presentation headroom phase입니다.
		const float OverspeedPhase = CFVehicleCameraMath::CalculateOverspeedPhase(
			CameraRuntimeState.SpeedRatio,
			DrivingFXConfig.MaxSpeedPresentationRatio);

		// 기존 Speed FOV Curve 출력에 초과속도 headroom만 추가한 최종 FOV 오프셋입니다.
		const float SpeedFOVOffsetDeg = bDrivingMotionInputValid ? FMath::Clamp(
			EvaluateFiniteCurve(DrivingFXConfig.SpeedFOVOffsetCurve, BaseSpeedCurveRatio)
				+ OverspeedPhase * FMath::Max(DrivingFXConfig.MaxOverspeedFOVBonusDeg, 0.0f),
			-FMath::Abs(DrivingFXConfig.MaxSpeedFOVOffsetDeg),
			FMath::Abs(DrivingFXConfig.MaxSpeedFOVOffsetDeg)) * ResolvedSpeedFXScale : 0.0f;

		// 기존 Speed Arm Curve 출력에 초과속도 headroom만 추가한 최종 Arm 오프셋입니다.
		const float SpeedArmOffsetCm = bDrivingMotionInputValid ? FMath::Clamp(
			EvaluateFiniteCurve(DrivingFXConfig.SpeedArmOffsetCurve, BaseSpeedCurveRatio)
				+ OverspeedPhase * FMath::Max(DrivingFXConfig.MaxOverspeedArmBonusCm, 0.0f),
			-FMath::Abs(DrivingFXConfig.MaxSpeedArmOffsetCm),
			FMath::Abs(DrivingFXConfig.MaxSpeedArmOffsetCm)) * ResolvedSpeedFXScale : 0.0f;

		// 기존 normalized 가속 Curve 출력에 USER feel 보강 배율을 적용한 Rear Kick입니다.
		const float AccelerationArmKickCm = bDrivingMotionInputValid ? FMath::Clamp(
			EvaluateFiniteCurve(DrivingFXConfig.AccelerationArmKickCurve, CameraRuntimeState.AccelerationRate)
				* FMath::Max(DrivingFXConfig.AccelerationRearKickScale, 0.0f),
			0.0f,
			FMath::Abs(DrivingFXConfig.MaxAccelerationArmKickCm)) * ResolvedMotionFXScale : 0.0f;
		// normalized 제동 Curve를 Clamp/상태감쇠까지 적용한 Forward Kick입니다.
		const float BrakingArmKickCm = bDrivingMotionInputValid ? FMath::Clamp(
			EvaluateFiniteCurve(DrivingFXConfig.BrakingArmKickCurve, CameraRuntimeState.BrakingRate),
			0.0f,
			FMath::Abs(DrivingFXConfig.MaxBrakingArmKickCm)) * ResolvedMotionFXScale : 0.0f;
		// normalized LateralRate Curve에서 얻은 차량 기준 기본 Roll 요구량입니다.
		const float LateralRollCurveOutputDeg = bDrivingMotionInputValid ? FMath::Clamp(
			EvaluateFiniteCurve(DrivingFXConfig.LateralRollCurve, CameraRuntimeState.LateralRate),
			-FMath::Abs(DrivingFXConfig.MaxLateralRollDeg),
			FMath::Abs(DrivingFXConfig.MaxLateralRollDeg)) : 0.0f;

		CameraRuntimeState.SpeedFOVOffsetDeg = SpeedFOVOffsetDeg;
		CameraRuntimeState.SpeedArmOffsetCm = SpeedArmOffsetCm;
		CameraRuntimeState.AccelerationArmKickCm = AccelerationArmKickCm;
		CameraRuntimeState.BrakingArmKickCm = BrakingArmKickCm;

		// WorldAimRotation 준비 후 실제 차체 Motion과 자유시점 정렬을 합성하기 전 차량 기준 Roll 요구량입니다.
		PresentationRollDeg = LateralRollCurveOutputDeg;

		PresentationDesiredArmLength += SpeedArmOffsetCm + AccelerationArmKickCm - BrakingArmKickCm;
		PresentationDesiredArmLength = FMath::Clamp(
			PresentationDesiredArmLength,
			CameraTuningConfig.MinArmLength,
			FMath::Max(CameraTuningConfig.MinArmLength, DrivingFXConfig.MaxPresentationArmLengthCm));
	}
	else
	{
		// legacy 전용 global speed bonus 계산에 사용할 현재 차량 속도(km/h)입니다.
		const float VehicleSpeedKmh = FMath::Max(0.0f, GetVehicleSpeedKmh());
		// legacy 전용 global SpeedForMaxBonusKmh 기반 0~1 속도 비율입니다.
		const float SpeedAlpha = CameraTuningConfig.SpeedForMaxBonusKmh > KINDA_SMALL_NUMBER
			? FMath::Clamp(VehicleSpeedKmh / CameraTuningConfig.SpeedForMaxBonusKmh, 0.0f, 1.0f)
			: 0.0f;
		if (CameraTuningConfig.bUseSpeedBasedArmLength)
		{
			PresentationDesiredArmLength += CameraTuningConfig.MaxSpeedArmLengthBonus * SpeedAlpha;
		}
		if (CameraTuningConfig.bUseSpeedBasedFOV)
		{
			PresentationDesiredFOV += CameraTuningConfig.MaxSpeedFOVBonus * SpeedAlpha;
		}
		PresentationDesiredArmLength = FMath::Max(CameraTuningConfig.MinArmLength, PresentationDesiredArmLength);
	}

	// 충돌 압축 비율 계산에 사용할 이전 frame 실제 해결 Arm 길이입니다.
	const float PreviousSolvedArmLength = CameraRuntimeState.SolvedArmLength > KINDA_SMALL_NUMBER
		? CameraRuntimeState.SolvedArmLength
		: CurrentArmLength;
	// 목표 Arm 대비 이전 frame 실제 충돌 해결 거리를 0~1로 정규화한 비율입니다.
	const float CollisionCompressionRatio = PresentationDesiredArmLength > KINDA_SMALL_NUMBER
		? FMath::Clamp(PreviousSolvedArmLength / PresentationDesiredArmLength, 0.0f, 1.0f)
		: 1.0f;

	if (CameraTuningConfig.bUseCollisionViewAssist && CollisionCompressionRatio < CameraTuningConfig.CollisionViewAssistStartRatio)
	{
		// 0으로 나누지 않도록 보정한 충돌 가시성 보조 시작 비율입니다.
		const float SafeCollisionAssistStartRatio = FMath::Max(KINDA_SMALL_NUMBER, CameraTuningConfig.CollisionViewAssistStartRatio);
		// 충돌 압축이 심할수록 1에 가까워지는 가시성 보조 강도입니다.
		const float CollisionAssistAlpha = 1.0f - FMath::Clamp(CollisionCompressionRatio / SafeCollisionAssistStartRatio, 0.0f, 1.0f);
		ResolvedHeightOffset += CameraTuningConfig.MaxCollisionHeightAssist * CollisionAssistAlpha;
		GameplayDesiredFOV += CameraTuningConfig.MaxCollisionFOVAssist * CollisionAssistAlpha;
		if (!bUseNormalizedDrivingFX)
		{
			PresentationDesiredFOV += CameraTuningConfig.MaxCollisionFOVAssist * CollisionAssistAlpha;
		}
	}

	if (bUseNormalizedDrivingFX)
	{
		PresentationDesiredFOV = GameplayDesiredFOV + CameraRuntimeState.SpeedFOVOffsetDeg;
		// 역전된 authored Clamp 값도 안전하게 해석한 Presentation FOV 하한입니다.
		const float MinPresentationFOVDeg = FMath::Min(DrivingFXConfig.MinPresentationFOVDeg, DrivingFXConfig.MaxPresentationFOVDeg);
		// 역전된 authored Clamp 값도 안전하게 해석한 Presentation FOV 상한입니다.
		const float MaxPresentationFOVDeg = FMath::Max(DrivingFXConfig.MinPresentationFOVDeg, DrivingFXConfig.MaxPresentationFOVDeg);
		PresentationDesiredFOV = FMath::Clamp(PresentationDesiredFOV, MinPresentationFOVDeg, MaxPresentationFOVDeg);
	}

	CurrentArmLength = FMath::FInterpTo(CurrentArmLength, PresentationDesiredArmLength, DeltaTime, CameraTuningConfig.ArmLengthInterpSpeed);
	if (bUseNormalizedDrivingFX)
	{
		CurrentGameplayFOV = FMath::FInterpTo(CurrentGameplayFOV, GameplayDesiredFOV, DeltaTime, CameraTuningConfig.FOVInterpSpeed);
		CurrentFOV = FMath::FInterpTo(CurrentFOV, PresentationDesiredFOV, DeltaTime, CameraTuningConfig.FOVInterpSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, PresentationDesiredFOV, DeltaTime, CameraTuningConfig.FOVInterpSpeed);
		CurrentGameplayFOV = CurrentFOV;
	}

	CameraRuntimeState.DesiredArmLength = PresentationDesiredArmLength;
	CameraRuntimeState.CurrentArmLength = CurrentArmLength;
	CameraRuntimeState.DesiredFOV = PresentationDesiredFOV;
	CameraRuntimeState.CurrentFOV = CurrentFOV;
	CameraRuntimeState.CurrentGameplayFOV = CurrentGameplayFOV;
	CameraRuntimeState.bCameraPitchRollIsolationApplied = bIsolateCameraFromVehiclePitchRoll;

	// 현재 CameraData를 반영한 월드 Pivot 위치입니다.
	const FVector PivotWorldLocation = GetPivotWorldLocation(CameraTuningConfig);
	// 차량 Pitch/Roll을 분리한 Gameplay 기준 Aim 회전입니다.
	const FRotator BaseVehicleAimRotation = GetBaseVehicleAimRotation();
	// Camera Yaw damping을 적용한 Presentation/Aim 공통 기본 회전입니다.
	const FRotator CameraBaseAimRotation = ResolveCameraYawDampedBaseRotation(DeltaTime, BaseVehicleAimRotation);
	// Clamp된 사용자 Aim을 합성한 최종 월드 Aim 회전입니다.
	const FRotator WorldAimRotation(
		CameraBaseAimRotation.Pitch + CameraRuntimeState.ClampedAimPitch,
		CameraBaseAimRotation.Yaw + CameraRuntimeState.ClampedAimYaw,
		0.0f);

	if (bUseNormalizedDrivingFX)
	{
		// 차량 Yaw-only 전방과 자유 조준이 합성된 현재 View 전방의 signed 수평 정렬값입니다.
		const float LateralViewAlignment = CFVehicleCameraMath::CalculateSignedViewAlignment(
			BaseVehicleAimRotation.Vector(),
			WorldAimRotation.Vector());

		// 실제 차체 Motion 강도, 자유시점 정렬, 기존 상태 감쇠를 합친 signed Lateral Presentation 배율입니다.
		const float LateralPresentationScale = CFVehicleCameraMath::CalculateLateralPresentationScale(
			CameraRuntimeState.LateralBodyMotionIntensity,
			LateralViewAlignment,
			CameraRuntimeState.ResolvedMotionFXScale);

		CameraRuntimeState.LateralViewAlignment = LateralViewAlignment;

		// 차량 기준 Lateral Curve 출력을 실제 차체 움직임과 화면 방향에 맞춰 최종 Presentation Roll로 변환합니다.
		PresentationRollDeg = FMath::Clamp(
			PresentationRollDeg * LateralPresentationScale,
			-FMath::Abs(DrivingFXConfig.MaxLateralRollDeg),
			FMath::Abs(DrivingFXConfig.MaxLateralRollDeg));

		if (!FMath::IsFinite(PresentationRollDeg))
		{
			PresentationRollDeg = 0.0f;
		}

		CameraRuntimeState.LateralRollDeg = PresentationRollDeg;
	}
	else
	{
		CameraRuntimeState.LateralViewAlignment = 0.0f;
		CameraRuntimeState.LateralRollDeg = 0.0f;
	}

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
		// SpringArm 자체 충돌과 별개로 full desired path를 검사해 경계 chatter를 억제한 실제 Target Arm입니다.
		const float CollisionStableArmLength = ResolveCollisionStableArmLength(
			DeltaTime,
			CameraTuningConfig,
			PivotWorldLocation,
			WorldAimRotation,
			CurrentArmLength,
			ResolvedSideOffset,
			ResolvedHeightOffset);
		CameraBoom->TargetArmLength = CollisionStableArmLength;
		CameraBoom->SocketOffset = FVector(0.0f, ResolvedSideOffset, ResolvedHeightOffset);
	}
	else if (FollowCamera)
	{
		// SpringArm이 없을 때 직접 배치할 최종 Camera 월드 위치입니다.
		const FVector ManualCameraLocation = PivotWorldLocation - (WorldAimRotation.Vector() * CurrentArmLength)
			+ WorldAimRotation.RotateVector(FVector(0.0f, ResolvedSideOffset, ResolvedHeightOffset));
		FollowCamera->SetWorldLocationAndRotation(ManualCameraLocation, WorldAimRotation);
	}

	CurrentGameplayView = FCFVehicleCameraGameplayView();
	if (FollowCamera)
	{
		// 신규 Driving Roll을 넣기 전 actual Camera orientation을 Gameplay View로 캡처합니다.
		// 신규 Driving Roll을 적용하기 전 Gameplay 기준 실제 Camera 월드 회전입니다.
		const FRotator PreDrivingCameraRotation = FollowCamera->GetComponentRotation();
		CurrentGameplayView.bValid = true;
		CurrentGameplayView.ViewOrigin = FollowCamera->GetComponentLocation();
		CurrentGameplayView.ViewDirection = PreDrivingCameraRotation.Vector();
		CurrentGameplayView.ViewUpDirection = PreDrivingCameraRotation.RotateVector(FVector::UpVector);
		CurrentGameplayView.VerticalFOVDeg = CurrentGameplayFOV;

		FollowCamera->SetFieldOfView(CurrentFOV);

		if (bUseNormalizedDrivingFX && !FMath::IsNearlyZero(PresentationRollDeg))
		{
			if (CameraBoom && bHasBaseFollowCameraRelativeRotation)
			{
				const FRotator PresentationRelativeRotation(
					BaseFollowCameraRelativeRotation.Pitch,
					BaseFollowCameraRelativeRotation.Yaw,
					BaseFollowCameraRelativeRotation.Roll + PresentationRollDeg);
				FollowCamera->SetRelativeRotation(PresentationRelativeRotation);
			}
			else
			{
				FollowCamera->SetWorldRotation(FRotator(
					PreDrivingCameraRotation.Pitch,
					PreDrivingCameraRotation.Yaw,
					PreDrivingCameraRotation.Roll + PresentationRollDeg));
			}
			bPresentationRollAppliedLastFrame = true;
		}

		CameraRuntimeState.SolvedArmLength = FVector::Distance(PivotWorldLocation, FollowCamera->GetComponentLocation());
	}
	else
	{
		CameraRuntimeState.SolvedArmLength = CurrentArmLength;
	}

}

// [v0.4.1] 전체 Presentation Arm 경로를 독립 Sweep으로 검사해 SpringArm collision 경계의 hit/clear 반복을 안정화합니다.
float UCFVehicleCameraComp::ResolveCollisionStableArmLength(
	float DeltaTime,
	const FCFVehicleCameraTuningConfig& CameraTuningConfig,
	const FVector& PivotWorldLocation,
	const FRotator& WorldAimRotation,
	float DesiredArmLength,
	float ResolvedSideOffset,
	float ResolvedHeightOffset)
{
	// authored 최소 Arm보다 작지 않도록 제한한 이번 frame Presentation 목표 Arm입니다.
	const float SafeDesiredArmLength = FMath::Max(CameraTuningConfig.MinArmLength, DesiredArmLength);

	if (!CameraBoom || !CameraTuningConfig.bEnableBoomCollisionTest)
	{
		CollisionRecoveryArmLength = SafeDesiredArmLength;
		CollisionClearElapsedSec = 0.0f;
		bCollisionRecoveryActive = false;
		return SafeDesiredArmLength;
	}

	// collision clearance를 검사할 현재 World입니다.
	UWorld* CurrentWorld = GetWorld();
	// Sweep에서 자기 차량을 제외할 Owner Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (!CurrentWorld || !OwnerActor)
	{
		CollisionRecoveryArmLength = SafeDesiredArmLength;
		CollisionClearElapsedSec = 0.0f;
		bCollisionRecoveryActive = false;
		return SafeDesiredArmLength;
	}

	// SpringArm TargetOffset을 포함한 collision Sweep 시작점입니다. TargetOffset은 월드 공간 오프셋입니다.
	const FVector SweepStart = PivotWorldLocation + CameraBoom->TargetOffset;
	// 이번 frame의 전체 Presentation Arm과 SocketOffset을 반영한 collision Sweep 목표점입니다.
	const FVector DesiredSocketOffsetWorld = WorldAimRotation.RotateVector(FVector(0.0f, ResolvedSideOffset, ResolvedHeightOffset));
	// Presentation 목표 Arm이 완전히 펼쳐졌을 때의 카메라 목표 위치입니다.
	const FVector SweepEnd = SweepStart - (WorldAimRotation.Vector() * SafeDesiredArmLength) + DesiredSocketOffsetWorld;

	// 충돌 복귀 중에는 경계에서 Clear/Hit가 반복되지 않도록 약간 더 넓게 검사할 추가 Probe 반경입니다.
	const float RecoveryProbePaddingCm = bCollisionRecoveryActive
		? FMath::Max(0.0f, CameraTuningConfig.CollisionReleaseProbePaddingCm)
		: 0.0f;
	// 최종 Sweep Sphere 반경입니다.
	const float SweepProbeRadiusCm = FMath::Max(1.0f, CameraTuningConfig.CollisionProbeSize + RecoveryProbePaddingCm);

	// 카메라 collision Sweep 결과입니다.
	FHitResult CollisionHitResult;
	// 자기 차량은 카메라 collision obstacle로 취급하지 않는 Query 설정입니다.
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(CFVehicleCameraCollisionRecovery), false, OwnerActor);
	// 전체 Presentation 경로가 현재 obstacle에 막혀 있는지 확인합니다.
	const bool bHasBlockingCollision = CurrentWorld->SweepSingleByChannel(
		CollisionHitResult,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_Camera,
		FCollisionShape::MakeSphere(SweepProbeRadiusCm),
		CollisionQueryParams);

	if (bHasBlockingCollision)
	{
		CollisionClearElapsedSec = 0.0f;

		// Sweep hit fraction을 Presentation Arm 길이에 대응시킨 즉시 안전 Arm 추정값입니다.
		const float HitSafeArmLength = FMath::Clamp(
			SafeDesiredArmLength * FMath::Clamp(CollisionHitResult.Time, 0.0f, 1.0f),
			CameraTuningConfig.MinArmLength,
			SafeDesiredArmLength);

		if (!bCollisionRecoveryActive)
		{
			CollisionRecoveryArmLength = HitSafeArmLength;
			bCollisionRecoveryActive = true;
		}
		else
		{
			// 충돌이 계속되는 동안에는 더 안쪽으로 필요한 변화만 즉시 허용하고 바깥쪽 재확장은 금지합니다.
			CollisionRecoveryArmLength = FMath::Min(CollisionRecoveryArmLength, HitSafeArmLength);
		}

		return FMath::Clamp(
			CollisionRecoveryArmLength,
			CameraTuningConfig.MinArmLength,
			SafeDesiredArmLength);
	}

	if (!bCollisionRecoveryActive)
	{
		CollisionRecoveryArmLength = SafeDesiredArmLength;
		CollisionClearElapsedSec = 0.0f;
		return SafeDesiredArmLength;
	}

	// 비정상 DeltaTime이 collision release 상태를 임의로 진행시키지 않게 한 안전 DeltaTime입니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime) && DeltaTime > 0.0f ? DeltaTime : 0.0f;
	CollisionClearElapsedSec += SafeDeltaTime;

	// hit/clear 경계 chatter를 막기 위해 복귀를 시작하기 전에 요구할 연속 Clear 시간입니다.
	const float SafeReleaseHoldTimeSec = FMath::Max(0.0f, CameraTuningConfig.CollisionReleaseHoldTimeSec);
	if (CollisionClearElapsedSec < SafeReleaseHoldTimeSec)
	{
		return FMath::Clamp(
			CollisionRecoveryArmLength,
			CameraTuningConfig.MinArmLength,
			SafeDesiredArmLength);
	}

	// 충분한 Clear가 확인된 뒤 원래 Presentation Arm으로 복귀할 보간 속도입니다.
	const float SafeRecoveryInterpSpeed = FMath::Max(0.0f, CameraTuningConfig.CollisionRecoveryInterpSpeed);
	CollisionRecoveryArmLength = SafeRecoveryInterpSpeed > KINDA_SMALL_NUMBER
		? FMath::FInterpTo(CollisionRecoveryArmLength, SafeDesiredArmLength, SafeDeltaTime, SafeRecoveryInterpSpeed)
		: SafeDesiredArmLength;

	if (FMath::IsNearlyEqual(CollisionRecoveryArmLength, SafeDesiredArmLength, 0.5f))
	{
		CollisionRecoveryArmLength = SafeDesiredArmLength;
		CollisionClearElapsedSec = 0.0f;
		bCollisionRecoveryActive = false;
	}

	return FMath::Clamp(
		CollisionRecoveryArmLength,
		CameraTuningConfig.MinArmLength,
		SafeDesiredArmLength);
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

			const FVector TraceStart = GetCurrentAimTraceStartLocation();
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
