// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.1
// Date: 2026-05-22
// Description: CarFight 차량 Aim 시스템의 복제 Aim 시각 상태 구현
// Scope: Owner Pawn과 VehicleCameraComp 참조 초기화, Tick 기반 Local Aim 상태 갱신, 서버 검증/복제 시각 상태 저장을 제공합니다.

#include "CFVehicleAimComp.h"

#include "CFVehicleCameraComp.h"
#include "CFVehiclePawn.h"
#include "Net/UnrealNetwork.h"

UCFVehicleAimComp::UCFVehicleAimComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bAimRuntimeReady = false;
	LastAimRuntimeSummary = TEXT("Constructed");
}

void UCFVehicleAimComp::BeginPlay()
{
	Super::BeginPlay();
	InitializeAimRuntime();
}

void UCFVehicleAimComp::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// [v1.3.1] 이 AimComp를 소유한 Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor && OwnerActor->GetNetMode() == NM_DedicatedServer)
	{
		LocalAimState.LocalReticleState = ECFVehicleReticleState::Hidden;
		return;
	}

	RefreshLocalAimState(DeltaTime);
}

// [v1.3.0] 복제 대상 Aim 시각 상태를 네트워크 복제 목록에 등록합니다.
void UCFVehicleAimComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCFVehicleAimComp, RepAimVisualState);
}

bool UCFVehicleAimComp::InitializeAimRuntime()
{
	bAimRuntimeReady = false;
	LastAimRuntimeSummary = TEXT("AimRuntime: InitializeStarted");

	// [v1.0.0] Owner Pawn과 VehicleCameraComp 참조가 모두 준비되었는지 여부입니다.
	const bool bReferencesReady = RefreshAimRuntimeReferences();
	bAimRuntimeReady = bReferencesReady;

	LastAimRuntimeSummary = FString::Printf(
		TEXT("AimRuntime: Owner=%s, Camera=%s, Ready=%s"),
		OwnerVehiclePawn ? TEXT("Ready") : TEXT("Missing"),
		CachedVehicleCameraComp ? TEXT("Ready") : TEXT("Missing"),
		bAimRuntimeReady ? TEXT("True") : TEXT("False"));

	return bAimRuntimeReady;
}

bool UCFVehicleAimComp::RefreshAimRuntimeReferences()
{
	OwnerVehiclePawn = ResolveOwnerVehiclePawn();
	CachedVehicleCameraComp = ResolveVehicleCameraComp();

	// [v1.0.0] Owner 차량 Pawn 참조를 찾았는지 여부입니다.
	const bool bHasOwnerVehiclePawn = (OwnerVehiclePawn != nullptr);

	// [v1.0.0] Owner 차량 Pawn에서 VehicleCameraComp를 찾았는지 여부입니다.
	const bool bHasVehicleCameraComp = (CachedVehicleCameraComp != nullptr);
	LastAimRuntimeSummary = FString::Printf(
		TEXT("AimRuntime: RefreshReferences Owner=%s, Camera=%s"),
		bHasOwnerVehiclePawn ? TEXT("Ready") : TEXT("Missing"),
		bHasVehicleCameraComp ? TEXT("Ready") : TEXT("Missing"));

	return bHasOwnerVehiclePawn && bHasVehicleCameraComp;
}

FCFVehicleLocalAimState UCFVehicleAimComp::GetLocalAimState() const
{
	return LocalAimState;
}

ECFVehicleReticleState UCFVehicleAimComp::GetReticleState() const
{
	return LocalAimState.LocalReticleState;
}

FCFVehicleServerAimState UCFVehicleAimComp::GetServerAimState() const
{
	return ServerAimState;
}

FCFVehicleRepAimVisualState UCFVehicleAimComp::GetRepAimVisualState() const
{
	return RepAimVisualState;
}

FCFVehicleAimProfile UCFVehicleAimComp::GetDefaultAimProfile() const
{
	return DefaultAimProfile;
}

bool UCFVehicleAimComp::IsAimRuntimeReady() const
{
	return bAimRuntimeReady;
}

FString UCFVehicleAimComp::GetLastAimRuntimeSummary() const
{
	return LastAimRuntimeSummary;
}

FCFVehicleFireRequest UCFVehicleAimComp::BuildFireRequest(const int32 FireRequestId, const float ClientFireTimeSeconds) const
{
	// [v1.2.0] 서버로 전달할 발사 요청 결과입니다.
	FCFVehicleFireRequest FireRequest;
	FireRequest.FireRequestId = FireRequestId;
	FireRequest.ClientFireTimeSeconds = ClientFireTimeSeconds;
	FireRequest.PredictedAimTargetLocation = LocalAimState.LocalAimTargetLocation;
	FireRequest.AimDirection = LocalAimState.LocalAimDirection;
	FireRequest.WeaponGroupId = DefaultAimProfile.ProfileName;

	if (OwnerVehiclePawn)
	{
		FireRequest.AimOrigin = OwnerVehiclePawn->GetActorLocation();
	}

	return FireRequest;
}

void UCFVehicleAimComp::ApplyServerFireResult(const FCFVehicleFireResult& FireResult)
{
	ServerAimState.ServerAimTargetLocation = FireResult.ServerAimTargetLocation;
	ServerAimState.bServerWithinWeaponArc = FireResult.bAccepted || FireResult.RejectReason != ECFVehicleFireRejectReason::OutOfWeaponArc;
	ServerAimState.bServerCanFire = FireResult.bAccepted;
	ServerAimState.LastServerRejectReason = FireResult.RejectReason;

	if (FireResult.bAccepted)
	{
		ServerAimState.LastAcceptedFireRequestId = FireResult.FireRequestId;
	}
	else
	{
		ServerAimState.LastRejectedFireRequestId = FireResult.FireRequestId;
	}
}

// [v1.3.0] 서버 발사 결과를 다른 클라이언트 표시용 복제 Aim 시각 상태에 반영합니다.
void UCFVehicleAimComp::UpdateRepAimVisualFromFireResult(const FCFVehicleFireRequest& FireRequest, const FCFVehicleFireResult& FireResult)
{
	// [v1.3.0] 이 컴포넌트를 소유한 Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	// [v1.3.0] 복제 표시용으로 정규화할 요청 조준 방향입니다.
	FVector ResolvedAimDirection = FVector(FireRequest.AimDirection);
	if (ResolvedAimDirection.IsNearlyZero())
	{
		// [v1.3.0] 요청 방향이 비정상일 때 사용할 서버 로컬 Aim 방향 fallback입니다.
		const FVector LocalAimDirection = FVector(LocalAimState.LocalAimDirection);
		ResolvedAimDirection = LocalAimDirection.IsNearlyZero() ? FVector::ForwardVector : LocalAimDirection;
	}
	ResolvedAimDirection = ResolvedAimDirection.GetSafeNormal();

	// [v1.3.0] 복제 표시용으로 사용할 서버 확정 목표 위치입니다.
	FVector ResolvedTargetLocation = FVector(FireResult.ServerAimTargetLocation);
	if (ResolvedTargetLocation.IsNearlyZero())
	{
		ResolvedTargetLocation = FVector(FireResult.ServerHitLocation);
	}
	if (ResolvedTargetLocation.IsNearlyZero())
	{
		ResolvedTargetLocation = FVector(FireRequest.PredictedAimTargetLocation);
	}

	RepAimVisualState.RepAimDirection = ResolvedAimDirection;
	RepAimVisualState.RepAimTargetLocation = ResolvedTargetLocation;
	RepAimVisualState.bIsFiringVisual = FireResult.bAccepted;
	RepAimVisualState.RepWeaponVisualMode = FireRequest.WeaponGroupId;
}

void UCFVehicleAimComp::BuildServerAimStateFromFireRequest(const FCFVehicleFireRequest& FireRequest, const ECFVehicleFireRejectReason RejectReason, const bool bAccepted)
{
	ServerAimState.ServerAimTargetLocation = FireRequest.PredictedAimTargetLocation;
	ServerAimState.bServerWithinWeaponArc = IsFireRequestWithinDefaultProfile(FireRequest);
	ServerAimState.bServerCanFire = bAccepted;
	ServerAimState.LastServerRejectReason = RejectReason;

	if (bAccepted)
	{
		ServerAimState.LastAcceptedFireRequestId = FireRequest.FireRequestId;
	}
	else
	{
		ServerAimState.LastRejectedFireRequestId = FireRequest.FireRequestId;
	}
}

bool UCFVehicleAimComp::IsFireRequestWithinDefaultProfile(const FCFVehicleFireRequest& FireRequest) const
{
	// [v1.2.0] 서버 검증에 사용할 요청 조준 방향입니다.
	const FVector AimDirection = FVector(FireRequest.AimDirection);
	if (AimDirection.IsNearlyZero())
	{
		return false;
	}

	// [v1.2.0] 차량 로컬 기준 조준 Yaw 각도입니다.
	float AimYawDeg = 0.0f;

	// [v1.2.0] 차량 로컬 기준 조준 Pitch 각도입니다.
	float AimPitchDeg = 0.0f;

	// [v1.2.0] 요청 조준 방향을 차량 로컬 각도로 변환할 수 있었는지 여부입니다.
	const bool bHasAimAngles = CalculateAimAnglesRelativeToVehicle(AimDirection.GetSafeNormal(), AimYawDeg, AimPitchDeg);
	return bHasAimAngles && IsAimWithinDefaultProfile(AimYawDeg, AimPitchDeg);
}

// [v1.3.0] 복제 Aim 시각 상태 수신 시 디버그 요약만 갱신합니다.
void UCFVehicleAimComp::OnRep_RepAimVisualState()
{
	LastAimRuntimeSummary = FString::Printf(
		TEXT("AimRuntime: RepAimVisual Updated Firing=%s, Weapon=%s"),
		RepAimVisualState.bIsFiringVisual ? TEXT("True") : TEXT("False"),
		*RepAimVisualState.RepWeaponVisualMode.ToString());
}

void UCFVehicleAimComp::RefreshLocalAimState(const float DeltaSeconds)
{
	// [v1.1.0] 현재 호출에서 DeltaSeconds 인자를 의도적으로 보관하지 않음을 명확히 합니다.
	(void)DeltaSeconds;

	if (!bAimRuntimeReady)
	{
		// [v1.1.0] BeginPlay 순서나 BP 구성 변경으로 늦게 준비될 수 있는 참조를 다시 확인한 결과입니다.
		const bool bReferencesReady = RefreshAimRuntimeReferences();
		bAimRuntimeReady = bReferencesReady;
	}

	if (!bAimRuntimeReady || !OwnerVehiclePawn || !CachedVehicleCameraComp)
	{
		LocalAimState.LocalReticleState = ECFVehicleReticleState::Hidden;
		LocalAimState.bLocalCanFire = false;
		LocalAimState.bLocalWithinWeaponArc = false;
		LocalAimState.bLocalAimBlocked = false;
		LastAimRuntimeSummary = TEXT("AimRuntime: LocalAimSkipped MissingRuntimeReferences");
		return;
	}

	if (!OwnerVehiclePawn->IsLocallyControlled())
	{
		LocalAimState.LocalReticleState = ECFVehicleReticleState::Hidden;
		LocalAimState.bLocalCanFire = false;
		LocalAimState.bLocalWithinWeaponArc = false;
		LocalAimState.bLocalAimBlocked = false;
		LastAimRuntimeSummary = TEXT("AimRuntime: LocalAimSkipped NotLocallyControlled");
		return;
	}

	// [v1.1.0] CameraComp가 계산한 현재 카메라 런타임 스냅샷입니다.
	const FCFVehicleCameraRuntimeState CameraRuntimeState = CachedVehicleCameraComp->GetCameraRuntimeState();

	// [v1.1.0] Local Aim 계산의 기준이 되는 Owner 차량 월드 위치입니다.
	const FVector OwnerLocation = OwnerVehiclePawn->GetActorLocation();

	// [v1.1.0] CameraRuntimeState에서 읽은 현재 Aim Trace 목표 위치입니다.
	const FVector AimTargetLocation = CameraRuntimeState.AimHitLocation;

	// [v1.1.0] Owner 위치에서 Aim 목표 위치까지의 원본 월드 방향 벡터입니다.
	const FVector RawAimDirection = AimTargetLocation - OwnerLocation;

	// [v1.1.0] 유효한 목표 위치가 없을 때 사용할 차량 정면 방향 fallback입니다.
	const FVector FallbackAimDirection = OwnerVehiclePawn->GetActorForwardVector();

	// [v1.1.0] LocalAimState에 저장할 최종 정규화 조준 방향입니다.
	const FVector ResolvedAimDirection = RawAimDirection.IsNearlyZero()
		? FallbackAimDirection.GetSafeNormal()
		: RawAimDirection.GetSafeNormal();

	// [v1.1.0] 차량 로컬 기준 조준 Yaw 각도입니다.
	float AimYawDeg = 0.0f;

	// [v1.1.0] 차량 로컬 기준 조준 Pitch 각도입니다.
	float AimPitchDeg = 0.0f;

	// [v1.1.0] 월드 조준 방향을 차량 로컬 각도로 변환할 수 있었는지 여부입니다.
	const bool bHasAimAngles = CalculateAimAnglesRelativeToVehicle(ResolvedAimDirection, AimYawDeg, AimPitchDeg);

	// [v1.1.0] DefaultAimProfile 기준 현재 조준이 무기 조준각 안에 있는지 여부입니다.
	const bool bWithinWeaponArc = bHasAimAngles && IsAimWithinDefaultProfile(AimYawDeg, AimPitchDeg);

	// [v1.1.0] Camera Runtime State에서 전달된 현재 조준 가림 여부입니다.
	const bool bAimBlocked = CameraRuntimeState.bAimBlocked;

	// [v1.1.0] 로컬 예측 기준 발사 가능 여부입니다.
	const bool bCanFire = bWithinWeaponArc && !bAimBlocked;

	LocalAimState.LocalAimTargetLocation = AimTargetLocation;
	LocalAimState.LocalAimDirection = ResolvedAimDirection;
	LocalAimState.bLocalWithinWeaponArc = bWithinWeaponArc;
	LocalAimState.bLocalAimBlocked = bAimBlocked;
	LocalAimState.bLocalCanFire = bCanFire;
	LocalAimState.LocalReticleState = BuildLocalReticleState(bWithinWeaponArc, bAimBlocked, bCanFire);
}

bool UCFVehicleAimComp::CalculateAimAnglesRelativeToVehicle(const FVector& AimDirection, float& OutYawDeg, float& OutPitchDeg) const
{
	OutYawDeg = 0.0f;
	OutPitchDeg = 0.0f;

	if (!OwnerVehiclePawn || AimDirection.IsNearlyZero())
	{
		return false;
	}

	// [v1.1.0] 월드 조준 방향을 차량 Actor 로컬 공간으로 변환한 방향입니다.
	const FVector LocalAimDirection = OwnerVehiclePawn->GetActorTransform().InverseTransformVectorNoScale(AimDirection).GetSafeNormal();
	if (LocalAimDirection.IsNearlyZero())
	{
		return false;
	}

	// [v1.1.0] Pitch 계산에서 수평면 길이로 사용할 X/Y 평면 방향 크기입니다.
	const float HorizontalLength = FVector2D(LocalAimDirection.X, LocalAimDirection.Y).Size();

	OutYawDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalAimDirection.Y, LocalAimDirection.X));
	OutPitchDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalAimDirection.Z, HorizontalLength));
	return true;
}

bool UCFVehicleAimComp::IsAimWithinDefaultProfile(const float AimYawDeg, const float AimPitchDeg) const
{
	// [v1.1.0] DefaultAimProfile의 Yaw 제한을 만족하는지 여부입니다.
	const bool bWithinYaw = AimYawDeg >= DefaultAimProfile.MinYawDeg && AimYawDeg <= DefaultAimProfile.MaxYawDeg;

	// [v1.1.0] DefaultAimProfile의 Pitch 제한을 만족하는지 여부입니다.
	const bool bWithinPitch = AimPitchDeg >= DefaultAimProfile.MinPitchDeg && AimPitchDeg <= DefaultAimProfile.MaxPitchDeg;

	return bWithinYaw && bWithinPitch;
}

ECFVehicleReticleState UCFVehicleAimComp::BuildLocalReticleState(const bool bWithinWeaponArc, const bool bAimBlocked, const bool bCanFire) const
{
	if (!bAimRuntimeReady)
	{
		return ECFVehicleReticleState::Hidden;
	}
	if (bAimBlocked)
	{
		return ECFVehicleReticleState::Blocked;
	}
	if (!bWithinWeaponArc)
	{
		return ECFVehicleReticleState::OutOfArc;
	}
	if (bCanFire)
	{
		return ECFVehicleReticleState::Ready;
	}
	return ECFVehicleReticleState::Hidden;
}

ACFVehiclePawn* UCFVehicleAimComp::ResolveOwnerVehiclePawn() const
{
	return Cast<ACFVehiclePawn>(GetOwner());
}

UCFVehicleCameraComp* UCFVehicleAimComp::ResolveVehicleCameraComp() const
{
	// [v1.0.0] 캐시된 Owner Pawn이 있으면 재사용하고, 없으면 Owner Actor에서 다시 해석한 차량 Pawn입니다.
	const ACFVehiclePawn* ResolvedOwnerVehiclePawn = OwnerVehiclePawn ? OwnerVehiclePawn.Get() : ResolveOwnerVehiclePawn();
	if (!ResolvedOwnerVehiclePawn)
	{
		return nullptr;
	}

	return ResolvedOwnerVehiclePawn->GetVehicleCameraComp();
}
