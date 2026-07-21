// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.11.0
// Date: 2026-07-14
// Description: CarFight 싱글플레이 차량 Aim 시스템 구현
// Changelog:
// - v1.11.0: 터렛 정책에 따라 정렬 중 bLocalCanFire를 허용하고 TurretAligning Reticle 상태는 독립적으로 유지.
// - v1.10.1: 정상 정렬 대기 경로가 Hidden으로 떨어지지 않도록 TurretAligning Reticle 우선순위를 추가.
// - v1.10.0: Weapon Aim Solution 저장/조회와 LocalAimState 반영 로직을 추가.
// - v1.9.0: 터렛 안정화 전 발사 정책에 맞춰 조준각 초과를 표시 상태로만 남기고 로컬 발사 예측 차단에서 제외.
// - v1.8.0: 발사 검증/시각 상태 저장과 갱신 경로를 FireValidationState / AimVisualState 명칭으로 교체.
// - v1.6.0: 로컬 Fire Command 전환에 맞춰 발사 요청/결과 처리 설명과 Aim 시각 상태 갱신 조건을 정리.
// - v1.5.0: 싱글플레이 기준선에서 Aim 시각 상태의 UE 복제 등록과 OnRep 경로를 제거.
// - v1.4.0: 싱글플레이 전환에 맞춰 AimComp 기본 컴포넌트 복제를 비활성화.
// Migration:
// - bLocalCanFire는 유효한 Weapon Aim Solution, 유효한 최종 AimDirection, MuzzleBlocked=false, 정렬 정책을 함께 반영한다.
// - BuildLocalReticleState는 Runtime 미준비, MuzzleBlocked, OutOfArc, TurretAligning, Ready, Hidden 순서로 상태를 해석한다.
// - Pawn이 SetWeaponAimSolution으로 전달한 MuzzleBlocked/정렬 상태를 LocalAimState의 CanFire/Blocked/ReticleState에 반영한다.
// - OutOfWeaponArc는 호환용 상태로 유지하지만, 기본 로컬 발사 검증에서는 조준각 초과를 단독 거부 사유로 쓰지 않는다.
// - GetServerAimState / ApplyServerFireResult 계열 호출은 FireValidationState 명칭 함수로 교체한다.
// - GetRepAimVisualState / UpdateRepAimVisualFromFireResult 호출은 AimVisualState 명칭 함수로 교체한다.
// - 멀티플레이 Aim 시각 상태 복제가 다시 필요하면 별도 멀티플레이 브랜치에서 복제 경로를 복구한다.
// Scope: Owner Pawn과 VehicleCameraComp 참조 초기화, Tick 기반 Local Aim 상태 갱신, 로컬 발사 검증/시각 상태 저장을 제공합니다.

#include "CFVehicleAimComp.h"

#include "CFVehicleCameraComp.h"
#include "CFVehiclePawn.h"

UCFVehicleAimComp::UCFVehicleAimComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
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

FCFVehicleFireValidationState UCFVehicleAimComp::GetFireValidationState() const
{
	return FireValidationState;
}

FCFVehicleAimVisualState UCFVehicleAimComp::GetAimVisualState() const
{
	return AimVisualState;
}

FCFVehicleWeaponAimSolution UCFVehicleAimComp::GetWeaponAimSolution() const
{
	return WeaponAimSolution;
}

void UCFVehicleAimComp::SetWeaponAimSolution(const FCFVehicleWeaponAimSolution& InWeaponAimSolution)
{
	WeaponAimSolution = InWeaponAimSolution;
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

// [v1.6.0] 현재 Local Aim 상태를 기준으로 로컬 발사 명령 데이터를 생성합니다.
FCFVehicleFireRequest UCFVehicleAimComp::BuildFireRequest(const int32 FireRequestId, const float ClientFireTimeSeconds) const
{
	// [v1.6.0] 로컬 검증에 전달할 발사 명령 결과입니다.
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

// [v1.8.0] 로컬 발사 처리 결과를 검증 상태에 반영합니다.
void UCFVehicleAimComp::ApplyFireValidationResult(const FCFVehicleFireResult& FireResult)
{
	FireValidationState.ValidationAimTargetLocation = FireResult.ValidationAimTargetLocation;
	if (FireResult.RejectReason == ECFVehicleFireRejectReason::OutOfWeaponArc)
	{
		FireValidationState.bValidationWithinWeaponArc = false;
	}
	FireValidationState.bValidationCanFire = FireResult.bAccepted;
	FireValidationState.LastValidationRejectReason = FireResult.RejectReason;

	if (FireResult.bAccepted)
	{
		FireValidationState.LastAcceptedFireRequestId = FireResult.FireRequestId;
	}
	else
	{
		FireValidationState.LastRejectedFireRequestId = FireResult.FireRequestId;
	}
}

// [v1.8.0] 로컬 발사 결과를 로컬 디버그/발사 결과 표시용 Aim 시각 상태에 반영합니다.
void UCFVehicleAimComp::UpdateAimVisualFromFireResult(const FCFVehicleFireRequest& FireRequest, const FCFVehicleFireResult& FireResult)
{
	// [v1.6.0] 이 컴포넌트를 소유한 Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// [v1.5.0] 표시용으로 정규화할 요청 조준 방향입니다.
	FVector ResolvedAimDirection = FVector(FireRequest.AimDirection);
	if (ResolvedAimDirection.IsNearlyZero())
	{
		// [v1.6.0] 요청 방향이 비정상일 때 사용할 로컬 Aim 방향 fallback입니다.
		const FVector LocalAimDirection = FVector(LocalAimState.LocalAimDirection);
		ResolvedAimDirection = LocalAimDirection.IsNearlyZero() ? FVector::ForwardVector : LocalAimDirection;
	}
	ResolvedAimDirection = ResolvedAimDirection.GetSafeNormal();

	// [v1.6.0] 표시용으로 사용할 로컬 확정 목표 위치입니다.
	FVector ResolvedTargetLocation = FVector(FireResult.ValidationAimTargetLocation);
	if (ResolvedTargetLocation.IsNearlyZero())
	{
		ResolvedTargetLocation = FVector(FireResult.LocalHitLocation);
	}
	if (ResolvedTargetLocation.IsNearlyZero())
	{
		ResolvedTargetLocation = FVector(FireRequest.PredictedAimTargetLocation);
	}

	AimVisualState.VisualAimDirection = ResolvedAimDirection;
	AimVisualState.VisualAimTargetLocation = ResolvedTargetLocation;
	AimVisualState.bIsFiringVisual = FireResult.bAccepted;
	AimVisualState.WeaponVisualMode = FireRequest.WeaponGroupId;
}

// [v1.8.0] 로컬 발사 명령과 검증 결과를 기준으로 검증 상태를 갱신합니다.
void UCFVehicleAimComp::BuildFireValidationStateFromFireCommand(const FCFVehicleFireRequest& FireCommand, const ECFVehicleFireRejectReason RejectReason, const bool bAccepted)
{
	FireValidationState.ValidationAimTargetLocation = FireCommand.PredictedAimTargetLocation;
	FireValidationState.bValidationWithinWeaponArc = IsFireRequestWithinDefaultProfile(FireCommand);
	FireValidationState.bValidationCanFire = bAccepted;
	FireValidationState.LastValidationRejectReason = RejectReason;

	if (bAccepted)
	{
		FireValidationState.LastAcceptedFireRequestId = FireCommand.FireRequestId;
	}
	else
	{
		FireValidationState.LastRejectedFireRequestId = FireCommand.FireRequestId;
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

// [v1.11.0] Weapon Aim Solution의 정책과 정렬 상태를 반영해 로컬 Aim 및 Reticle 상태를 갱신합니다.
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
		LocalAimState.bLocalAimTraceHasBlockingHit = false;
		LastAimRuntimeSummary = TEXT("AimRuntime: LocalAimSkipped MissingRuntimeReferences");
		return;
	}

	if (!OwnerVehiclePawn->IsLocallyControlled())
	{
		LocalAimState.LocalReticleState = ECFVehicleReticleState::Hidden;
		LocalAimState.bLocalCanFire = false;
		LocalAimState.bLocalWithinWeaponArc = false;
		LocalAimState.bLocalAimBlocked = false;
		LocalAimState.bLocalAimTraceHasBlockingHit = false;
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

	// [v1.10.0] Camera Runtime State에서 전달된 목표 표면 선택용 Blocking Hit 여부입니다.
	const bool bAimTraceHasBlockingHit = CameraRuntimeState.bAimTraceHasBlockingHit;

	// [v1.10.0] Weapon Aim Solution이 현재 로컬 조준 목표와 연결할 수 있는 유효 상태인지 여부입니다.
	const bool bHasWeaponAimSolution = WeaponAimSolution.bHasValidSolution;

	// [v1.10.0] Weapon Aim Solution 기준 실제 발사 차단 여부입니다.
	const bool bWeaponAimBlocked = bHasWeaponAimSolution && WeaponAimSolution.bMuzzleBlocked;

	// [v1.10.1] Weapon Aim Solution 기준 터렛 또는 총구가 아직 조준점에 정렬 중인지 여부입니다.
	const bool bWeaponIsAligning = bHasWeaponAimSolution
		&& (WeaponAimSolution.bTurretAligning || WeaponAimSolution.bWeaponNotAligned);

	// [v1.11.0] Weapon Aim Solution의 실제 최종 발사 방향이 유효한지 여부입니다.
	const bool bHasResolvedAimDirection = bHasWeaponAimSolution
		&& !WeaponAimSolution.AimDirection.ContainsNaN()
		&& !WeaponAimSolution.AimDirection.IsNearlyZero();

	// [v1.11.0] 정렬 상태가 현재 터렛 정책에서 발사를 차단하는지 여부입니다.
	const bool bAlignmentBlocksFire = bWeaponIsAligning && !WeaponAimSolution.bAllowFireWhileAligning;

	// [v1.11.0] 로컬 예측 기준 발사 가능 여부입니다.
	const bool bCanFire = bHasResolvedAimDirection
		&& !WeaponAimSolution.bMuzzleBlocked
		&& !bAlignmentBlocksFire;

	LocalAimState.LocalAimTargetLocation = AimTargetLocation;
	LocalAimState.LocalAimDirection = bHasWeaponAimSolution ? WeaponAimSolution.DesiredAimDirection.GetSafeNormal() : ResolvedAimDirection;
	LocalAimState.bLocalWithinWeaponArc = bWithinWeaponArc;
	LocalAimState.bLocalAimBlocked = bWeaponAimBlocked;
	LocalAimState.bLocalAimTraceHasBlockingHit = bAimTraceHasBlockingHit;
	LocalAimState.bLocalCanFire = bCanFire;
	LocalAimState.LocalReticleState = BuildLocalReticleState(bWithinWeaponArc, bWeaponAimBlocked, bWeaponIsAligning, bCanFire);
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

ECFVehicleReticleState UCFVehicleAimComp::BuildLocalReticleState(const bool bWithinWeaponArc, const bool bAimBlocked, const bool bWeaponIsAligning, const bool bCanFire) const
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
	if (bWeaponIsAligning)
	{
		return ECFVehicleReticleState::TurretAligning;
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
