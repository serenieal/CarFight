// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CF-FQ-030 Direct 미사일 PIE 전용 선택 가능 테스트 타겟 구현
// Scope: 경량 시각·충돌, 측면 왕복 Velocity, TargetSelectable과 자동 파괴를 구현합니다.
// Changelog:
// - v1.0.0: 미사일 전용 테스트 타겟 Actor 최초 추가.
// Migration:
// - MissileDirectTest 맵 외에는 자동 배치하거나 게임플레이 스폰 목록에 등록하지 않습니다.

#include "CFMissileTestTarget.h"

#include "CFCollisionChannels.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// [v1.0.0] 선택·충돌·시각 컴포넌트와 안전한 테스트 기본값을 초기화합니다.
ACFMissileTestTarget::ACFMissileTestTarget()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// [v1.0.0] TargetSelect Trace와 Projectile 적중에 사용할 구형 충돌 루트입니다.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(120.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetGenerateOverlapEvents(false);
	CollisionComponent->SetCanEverAffectNavigation(false);

	// [v1.0.0] 테스트 맵에서 쉽게 식별할 기본 큐브 시각 메시입니다.
	TargetMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMeshComponent"));
	TargetMeshComponent->SetupAttachment(CollisionComponent);
	TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetMeshComponent->SetGenerateOverlapEvents(false);
	TargetMeshComponent->SetCanEverAffectNavigation(false);
	TargetMeshComponent->SetRelativeScale3D(FVector(2.0f));

	// [v1.0.0] 외부 에셋 의존 없이 표시할 Engine 기본 Cube 메시 검색 결과입니다.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		TargetMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	ApplyTestCollisionResponses();
}

// [v1.0.0] 에디터 프로퍼티가 바뀐 뒤 충돌 응답과 초기 위치를 다시 적용합니다.
void ACFMissileTestTarget::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshTestTargetConfig();
}

// [v1.0.0] PIE 시작 위치와 자동 파괴 타이머를 준비합니다.
void ACFMissileTestTarget::BeginPlay()
{
	Super::BeginPlay();
	RefreshTestTargetConfig();

	if (bAutoDestroy && AutoDestroyDelaySeconds > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(
			AutoDestroyTimerHandle,
			this,
			&ACFMissileTestTarget::HandleAutoDestroyTimer,
			AutoDestroyDelaySeconds,
			false);
	}
}

// [v1.0.0] 측면 왕복 이동과 현재 목표 Velocity를 갱신합니다.
void ACFMissileTestTarget::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MovementMode != ECFMissileTestMoveMode::LateralPingPong)
	{
		CurrentTargetVelocity = FVector::ZeroVector;
		return;
	}

	// [v1.0.0] 비정상 DeltaTime과 음수 시간을 제거한 실제 이동 시간입니다.
	const float SafeDeltaSeconds = FMath::IsFinite(DeltaSeconds)
		? FMath::Max(DeltaSeconds, 0.0f)
		: 0.0f;

	// [v1.0.0] 0 벡터 입력을 월드 Y축으로 복구한 측면 이동 방향입니다.
	FVector SafeMoveDirection = LateralMoveDirection.GetSafeNormal();
	if (SafeMoveDirection.ContainsNaN() || SafeMoveDirection.IsNearlyZero())
	{
		SafeMoveDirection = FVector::RightVector;
	}

	// [v1.0.0] 음수와 비유한 값을 제거한 측면 이동 진폭입니다.
	const float SafeAmplitudeCm = FMath::IsFinite(LateralAmplitudeCm)
		? FMath::Max(LateralAmplitudeCm, 0.0f)
		: 0.0f;

	// [v1.0.0] 음수와 비유한 값을 제거한 중심 통과 최대 속력입니다.
	const float SafeSpeedCmPerSec = FMath::IsFinite(LateralSpeedCmPerSec)
		? FMath::Max(LateralSpeedCmPerSec, 0.0f)
		: 0.0f;
	if (SafeAmplitudeCm <= KINDA_SMALL_NUMBER || SafeSpeedCmPerSec <= KINDA_SMALL_NUMBER)
	{
		CurrentTargetVelocity = FVector::ZeroVector;
		return;
	}

	MotionElapsedSeconds += SafeDeltaSeconds;

	// [v1.0.0] 중심 통과 속력이 설정값과 같아지도록 계산한 사인 이동 각속도입니다.
	const float MotionAngularSpeedRadPerSec = SafeSpeedCmPerSec / SafeAmplitudeCm;

	// [v1.0.0] 현재 왕복 위상에서 이동 중심으로부터의 부호 포함 거리입니다.
	const float CurrentOffsetCm = FMath::Sin(MotionElapsedSeconds * MotionAngularSpeedRadPerSec) * SafeAmplitudeCm;

	// [v1.0.0] Guidance TargetVelocityEstimate에 제공할 현재 측면 이동 속도입니다.
	const float CurrentSpeedCmPerSec = FMath::Cos(MotionElapsedSeconds * MotionAngularSpeedRadPerSec) * SafeSpeedCmPerSec;

	CurrentTargetVelocity = SafeMoveDirection * CurrentSpeedCmPerSec;
	SetActorLocation(MotionCenterLocation + SafeMoveDirection * CurrentOffsetCm, false, nullptr, ETeleportType::TeleportPhysics);
}

// [v1.0.0] Guidance가 사용할 현재 테스트 타겟 Velocity를 반환합니다.
FVector ACFMissileTestTarget::GetVelocity() const
{
	return CurrentTargetVelocity;
}

// [v1.0.0] 테스트 타겟의 선택 가능 상태를 반환합니다.
bool ACFMissileTestTarget::IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const
{
	(void)SelectionContext;
	return IsValid(this) && !IsActorBeingDestroyed();
}

// [v1.0.0] HUD와 TargetSelect가 사용할 테스트 타겟 표시 정보를 반환합니다.
FCFTargetDisplayInfo ACFMissileTestTarget::GetTargetDisplayInfo_Implementation() const
{
	// [v1.0.0] TargetSelect HUD에 제공할 테스트 타겟 표시 정보입니다.
	FCFTargetDisplayInfo DisplayInfo;
	DisplayInfo.TargetId = TargetId.IsNone() ? GetFName() : TargetId;
	DisplayInfo.DisplayName = TargetDisplayName.IsEmpty()
		? FText::FromString(GetActorNameOrLabel())
		: TargetDisplayName;
	DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
	DisplayInfo.Relation = ECFTargetRelation::Hostile;
	DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
	DisplayInfo.AttributeTags.Add(TEXT("MissileTestTarget"));
	return DisplayInfo;
}

// [v1.0.0] 타겟 중심의 월드 위치를 선택 대표 위치로 반환합니다.
FVector ACFMissileTestTarget::GetTargetSelectionLocation_Implementation() const
{
	return CollisionComponent ? CollisionComponent->GetComponentLocation() : GetActorLocation();
}

// [v1.0.0] 파괴 전 테스트 타겟의 기본 추적 상태를 Visible로 반환합니다.
ECFTargetTrackState ACFMissileTestTarget::GetTargetTrackState_Implementation() const
{
	return IsActorBeingDestroyed() ? ECFTargetTrackState::Invalid : ECFTargetTrackState::Visible;
}

// [v1.0.0] 자동 파괴 대기 없이 현재 테스트 타겟을 즉시 제거합니다.
void ACFMissileTestTarget::DestroyTestTargetNow()
{
	Destroy();
}

// [v1.0.0] 현재 위치를 새 이동 중심으로 저장하고 왕복 시간과 Velocity를 초기화합니다.
void ACFMissileTestTarget::ResetTestMotion()
{
	MotionCenterLocation = GetActorLocation();
	MotionElapsedSeconds = 0.0f;
	CurrentTargetVelocity = FVector::ZeroVector;
}

// [v1.0.0] 에디터·Python 생성 뒤 현재 프로퍼티 기준으로 충돌과 이동 상태를 다시 적용합니다.
void ACFMissileTestTarget::RefreshTestTargetConfig()
{
	ApplyTestCollisionResponses();
	ResetTestMotion();
}

// [v1.0.0] Automation에서 월드 Tick 없이 목표 Velocity 추정값을 지정합니다.
void ACFMissileTestTarget::SetTestVelocityForAutomation(const FVector& InTargetVelocity)
{
	CurrentTargetVelocity = InTargetVelocity.ContainsNaN() ? FVector::ZeroVector : InTargetVelocity;
}

// [v1.0.0] Visibility, WeaponHit와 Projectile 채널 응답을 현재 설정에 맞게 적용합니다.
void ACFMissileTestTarget::ApplyTestCollisionResponses()
{
	if (!CollisionComponent)
	{
		return;
	}

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(CFCollisionChannels::WeaponHit, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(
		CFCollisionChannels::Projectile,
		bBlockProjectileHits ? ECR_Block : ECR_Ignore);
}

// [v1.0.0] 자동 파괴 타이머가 만료됐을 때 이 테스트 타겟을 제거합니다.
void ACFMissileTestTarget::HandleAutoDestroyTimer()
{
	DestroyTestTargetNow();
}
