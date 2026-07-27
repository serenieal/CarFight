// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-24
// Description: CarFight 타겟 선택 대표 위치 컴포넌트 구현
// Scope: 명시 TargetPoint, Actor Bounds 중심, Actor 위치 순서의 해석과 디버그 표시를 제공합니다.

#include "CFTargetPointComp.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	// [v1.0.0] 디버그 문자열에서 사용할 위치 출처 이름을 반환합니다.
	const TCHAR* GetTargetPointSourceName(const ECFTargetPointSource Source)
	{
		switch (Source)
		{
		case ECFTargetPointSource::TargetPoint:
			return TEXT("TargetPoint");
		case ECFTargetPointSource::ActorBounds:
			return TEXT("ActorBounds");
		case ECFTargetPointSource::ActorLocation:
			return TEXT("ActorLocation");
		default:
			return TEXT("Invalid");
		}
	}

	// [v1.0.0] 위치 출처별 디버그 표시 색상을 반환합니다.
	FColor GetTargetPointSourceColor(const ECFTargetPointSource Source)
	{
		switch (Source)
		{
		case ECFTargetPointSource::TargetPoint:
			return FColor::Cyan;
		case ECFTargetPointSource::ActorBounds:
			return FColor::Yellow;
		case ECFTargetPointSource::ActorLocation:
			return FColor::Orange;
		default:
			return FColor::Red;
		}
	}
}

UCFTargetPointComp::UCFTargetPointComp()
{
	PrimaryComponentTick.bCanEverTick = false;
		bAutoActivate = true;
	bUseAsTargetPoint = false;
	bAutoAlignToPreferredBounds = false;
	PreferredBoundsComponentName = NAME_None;
	PreferredBoundsLocalOffset = FVector::ZeroVector;
}

bool UCFTargetPointComp::AlignToPreferredBoundsComponent()
{
	AActor* OwnerActor = GetOwner();
	if (!bAutoAlignToPreferredBounds || !IsValid(OwnerActor) || PreferredBoundsComponentName.IsNone())
	{
		return false;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	OwnerActor->GetComponents(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent) || PrimitiveComponent->GetFName() != PreferredBoundsComponentName)
		{
			continue;
		}

		const FTransform ComponentTransform = PrimitiveComponent->GetComponentTransform();
		const FBoxSphereBounds ComponentBounds = PrimitiveComponent->CalcBounds(ComponentTransform);
		if (ComponentBounds.Origin.ContainsNaN())
		{
			return false;
		}

		const FVector LocalBoundsCenter = ComponentTransform.InverseTransformPosition(ComponentBounds.Origin);
		AttachToComponent(PrimitiveComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		SetRelativeLocation(LocalBoundsCenter + PreferredBoundsLocalOffset);
		return true;
	}

	return false;
}

bool UCFTargetPointComp::CanUseAsTargetPoint() const
{
	return IsValid(this)
		&& bUseAsTargetPoint
		&& IsActive();
}

FVector UCFTargetPointComp::GetTargetPointWorldLocation() const
{
	return GetComponentLocation();
}

FCFTargetPointResult UCFTargetPointComp::ResolveOwnerTargetPoint() const
{
	return ResolveTargetPoint(GetOwner());
}

FString UCFTargetPointComp::BuildTargetPointDebugSummary() const
{
	const FCFTargetPointResult Result = ResolveOwnerTargetPoint();
	return FString::Printf(
		TEXT("TargetPoint: Valid=%s Source=%s Location=%s"),
		Result.bIsValid ? TEXT("True") : TEXT("False"),
		GetTargetPointSourceName(Result.Source),
		*Result.WorldLocation.ToCompactString());
}

void UCFTargetPointComp::DrawTargetPointDebug(const float DurationSeconds, const float SphereRadius) const
{
	const AActor* OwnerActor = GetOwner();
	const UWorld* World = GetWorld();
	const FCFTargetPointResult Result = ResolveOwnerTargetPoint();
	if (!IsValid(OwnerActor) || !IsValid(World) || !Result.bIsValid)
	{
		return;
	}

	const FColor DebugColor = GetTargetPointSourceColor(Result.Source);
	DrawDebugSphere(World, Result.WorldLocation, FMath::Max(1.0f, SphereRadius), 16, DebugColor, false, FMath::Max(0.0f, DurationSeconds));
	DrawDebugLine(World, OwnerActor->GetActorLocation(), Result.WorldLocation, DebugColor, false, FMath::Max(0.0f, DurationSeconds), 0, 1.5f);
}

FCFTargetPointResult UCFTargetPointComp::ResolveTargetPoint(const AActor* TargetActor)
{
	FCFTargetPointResult Result;
	if (!IsValid(TargetActor))
	{
		return Result;
	}

	TInlineComponentArray<UCFTargetPointComp*> TargetPointComponents;
	TargetActor->GetComponents(TargetPointComponents);
	for (const UCFTargetPointComp* TargetPointComponent : TargetPointComponents)
	{
		if (!IsValid(TargetPointComponent) || !TargetPointComponent->CanUseAsTargetPoint())
		{
			continue;
		}

		const FVector TargetPointLocation = TargetPointComponent->GetTargetPointWorldLocation();
		if (!TargetPointLocation.ContainsNaN())
		{
			Result.bIsValid = true;
			Result.WorldLocation = TargetPointLocation;
			Result.Source = ECFTargetPointSource::TargetPoint;
			return Result;
		}
	}

	FVector BoundsOrigin = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(false, BoundsOrigin, BoundsExtent);
	if (!BoundsOrigin.ContainsNaN() && !BoundsExtent.IsNearlyZero())
	{
		Result.bIsValid = true;
		Result.WorldLocation = BoundsOrigin;
		Result.Source = ECFTargetPointSource::ActorBounds;
		return Result;
	}

	const FVector ActorLocation = TargetActor->GetActorLocation();
	if (!ActorLocation.ContainsNaN())
	{
		Result.bIsValid = true;
		Result.WorldLocation = ActorLocation;
		Result.Source = ECFTargetPointSource::ActorLocation;
	}

	return Result;
}
