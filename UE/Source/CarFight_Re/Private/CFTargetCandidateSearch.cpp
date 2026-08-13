// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.1
// Date: 2026-08-13
// Description: TS-P0-03 후보 탐색과 TS-P0-04 선택 수명 Tick 연결 구현
// Changelog:
// - v1.2.1: 기존 v1.2.0 의도와 달리 남아 있던 익명 네임스페이스 IsFiniteVector를 IsFiniteTargetCandidateVector로 실제 교정해 Unity Build 충돌을 제거.
// - v1.2.0: Unity 빌드에서 다른 구현 파일의 익명 네임스페이스 심볼과 충돌하지 않도록 후보 탐색 전용 이름으로 분리.
// Migration:
// - 후보 평가는 TargetSelect 전용 Trace와 ICFTargetSelectable 대표 위치를 사용한다.
// - 선택 확정 입력은 후속 Task에서 연결하며 Tick은 현재 후보 갱신과 이미 선택된 대상의 수명 검사만 수행한다.

#include "CFTargetSelectComp.h"

#include "CFCollisionChannels.h"
#include "CFTargetSelectable.h"
#include "CFVehicleCameraComp.h"
#include "CFVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "UnrealClient.h"

namespace
{
		const FName CandidateGetTargetDisplayInfoFunctionName(TEXT("GetTargetDisplayInfo"));
	constexpr float CandidateFloatTolerance = 0.0001f;

	bool IsFiniteTargetCandidateVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	bool HasCandidateBlueprintTargetSelectableOverride(const AActor* TargetActor, const FName FunctionName)
	{
		if (!IsValid(TargetActor))
		{
			return false;
		}

		const UFunction* TargetFunction = TargetActor->FindFunction(FunctionName);
		return IsValid(TargetFunction)
			&& TargetFunction->GetOuter() != UCFTargetSelectable::StaticClass();
	}

	FCFTargetDisplayInfo ResolveCandidateTargetDisplayInfo(const AActor* TargetActor)
	{
		if (HasCandidateBlueprintTargetSelectableOverride(TargetActor, CandidateGetTargetDisplayInfoFunctionName))
		{
			return ICFTargetSelectable::Execute_GetTargetDisplayInfo(TargetActor);
		}

		if (const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor))
		{
			return NativeTargetSelectable->GetTargetDisplayInfo_Implementation();
		}

		return ICFTargetSelectable::Execute_GetTargetDisplayInfo(TargetActor);
	}

	

	bool ContainsActor(const TArray<TObjectPtr<AActor>>& Actors, const AActor* TargetActor)
	{
		for (const TObjectPtr<AActor>& Actor : Actors)
		{
			if (Actor.Get() == TargetActor)
			{
				return true;
			}
		}

		return false;
	}

	FString GetCandidateActorSortText(const FCFTargetCandidate& Candidate)
	{
		const AActor* CandidateActor = Candidate.TargetActor.Get();
		return CandidateActor ? CandidateActor->GetPathName() : FString();
	}

	struct FCFTargetCandidateLess
	{
		bool operator()(const FCFTargetCandidate& Left, const FCFTargetCandidate& Right) const
		{
			if (Left.bDirectAimHit != Right.bDirectAimHit)
			{
				return Left.bDirectAimHit;
			}

			if (!FMath::IsNearlyEqual(Left.NormalizedScreenDistance, Right.NormalizedScreenDistance, CandidateFloatTolerance))
			{
				return Left.NormalizedScreenDistance < Right.NormalizedScreenDistance;
			}

			if (!FMath::IsNearlyEqual(Left.WorldDistanceCm, Right.WorldDistanceCm, CandidateFloatTolerance))
			{
				return Left.WorldDistanceCm < Right.WorldDistanceCm;
			}

			const int32 StableKeyComparison = Left.StableSortKey.ToString().Compare(Right.StableSortKey.ToString(), ESearchCase::CaseSensitive);
			if (StableKeyComparison != 0)
			{
				return StableKeyComparison < 0;
			}

			return GetCandidateActorSortText(Left).Compare(GetCandidateActorSortText(Right), ESearchCase::CaseSensitive) < 0;
		}
	};
}

void UCFTargetSelectComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld || CurrentWorld->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const float SafeDeltaTime = FMath::Max(0.0f, DeltaTime);
	const float RefreshIntervalSeconds = FMath::Max(0.001f, GetResolvedTargetSelectConfig().CandidateRefreshIntervalSec);

	SelectedTargetLifetimeCheckElapsedSeconds += SafeDeltaTime;
	if (SelectedTargetLifetimeCheckElapsedSeconds >= RefreshIntervalSeconds)
	{
		const float LifetimeDeltaTime = SelectedTargetLifetimeCheckElapsedSeconds;
		SelectedTargetLifetimeCheckElapsedSeconds = 0.0f;
		RefreshSelectedTargetLifetime(LifetimeDeltaTime);
	}

	if (!bAutoRefreshCandidate)
	{
		return;
	}

	ACFVehiclePawn* OwnerVehiclePawn = Cast<ACFVehiclePawn>(GetOwner());
	if (!OwnerVehiclePawn || !OwnerVehiclePawn->IsLocallyControlled())
	{
		return;
	}

	CandidateRefreshElapsedSeconds += SafeDeltaTime;
	if (CandidateRefreshElapsedSeconds < RefreshIntervalSeconds)
	{
		return;
	}

	CandidateRefreshElapsedSeconds = 0.0f;
	RefreshCurrentCandidate();

	if (bDrawCandidateSearchDebug)
	{
		DrawCandidateSearchDebug(0.0f, 20.0f);
	}
}

float UCFTargetSelectComp::CalculateNormalizedScreenDistance(const FCFTargetSearchView& SearchView, const FVector& TargetWorldLocation)
{
	if (!IsFiniteTargetCandidateVector(SearchView.ViewOrigin)
		|| !IsFiniteTargetCandidateVector(SearchView.ViewDirection)
		|| !IsFiniteTargetCandidateVector(SearchView.ViewUpDirection)
		|| !IsFiniteTargetCandidateVector(TargetWorldLocation)
		|| !FMath::IsFinite(SearchView.VerticalFOVDeg)
		|| !FMath::IsFinite(SearchView.ViewportAspectRatio))
	{
		return BIG_NUMBER;
	}

	const FVector ViewForward = SearchView.ViewDirection.GetSafeNormal();
	if (ViewForward.IsNearlyZero())
	{
		return BIG_NUMBER;
	}

	FVector ViewUp = SearchView.ViewUpDirection - ViewForward * FVector::DotProduct(SearchView.ViewUpDirection, ViewForward);
	if (!ViewUp.Normalize())
	{
		ViewUp = FVector::UpVector - ViewForward * FVector::DotProduct(FVector::UpVector, ViewForward);
		if (!ViewUp.Normalize())
		{
			ViewUp = FVector::RightVector - ViewForward * FVector::DotProduct(FVector::RightVector, ViewForward);
			if (!ViewUp.Normalize())
			{
				return BIG_NUMBER;
			}
		}
	}

	const FVector ViewRight = FVector::CrossProduct(ViewUp, ViewForward).GetSafeNormal();
	if (ViewRight.IsNearlyZero())
	{
		return BIG_NUMBER;
	}

	const FVector ToTarget = TargetWorldLocation - SearchView.ViewOrigin;
	const float ForwardDepth = FVector::DotProduct(ToTarget, ViewForward);
	if (ForwardDepth <= KINDA_SMALL_NUMBER)
	{
		return BIG_NUMBER;
	}

	const float SafeVerticalFOVDeg = FMath::Clamp(SearchView.VerticalFOVDeg, 1.0f, 179.0f);
	const float SafeAspectRatio = FMath::Max(0.1f, SearchView.ViewportAspectRatio);
	const float VerticalHalfTangent = FMath::Tan(FMath::DegreesToRadians(SafeVerticalFOVDeg * 0.5f));
	const float HorizontalHalfTangent = VerticalHalfTangent * SafeAspectRatio;
	if (VerticalHalfTangent <= KINDA_SMALL_NUMBER || HorizontalHalfTangent <= KINDA_SMALL_NUMBER)
	{
		return BIG_NUMBER;
	}

	const float NormalizedHorizontal = FVector::DotProduct(ToTarget, ViewRight) / (ForwardDepth * HorizontalHalfTangent);
	const float NormalizedVertical = FVector::DotProduct(ToTarget, ViewUp) / (ForwardDepth * VerticalHalfTangent);
	return FMath::Sqrt(NormalizedHorizontal * NormalizedHorizontal + NormalizedVertical * NormalizedVertical);
}

FCFTargetSearchResult UCFTargetSelectComp::EvaluateCandidateActors(const TArray<AActor*>& CandidateActors, const FCFTargetSearchView& SearchView, const FCFTargetSelectionContext& SelectionContext) const
{
	FCFTargetSearchResult SearchResult;
	SearchResult.InputActorCount = CandidateActors.Num();

	const FCFTargetSelectConfig Config = GetResolvedTargetSelectConfig();
	const FVector ViewDirection = SearchView.ViewDirection.GetSafeNormal();
	if (ViewDirection.IsNearlyZero() || !IsFiniteTargetCandidateVector(SearchView.ViewOrigin))
	{
		return SearchResult;
	}

	for (AActor* CandidateActor : CandidateActors)
	{
		if (!CanUseActorAsTarget(CandidateActor, SelectionContext))
		{
			continue;
		}

		const FVector TargetWorldLocation = ResolveTargetSelectionLocation(CandidateActor);
		if (!IsFiniteTargetCandidateVector(TargetWorldLocation))
		{
			continue;
		}

		const FVector ToTarget = TargetWorldLocation - SearchView.ViewOrigin;
		const float WorldDistanceCm = ToTarget.Size();
		const FVector DirectionToTarget = ToTarget.GetSafeNormal();
		const float AimDot = DirectionToTarget.IsNearlyZero()
			? 0.0f
			: FMath::Clamp(FVector::DotProduct(ViewDirection, DirectionToTarget), -1.0f, 1.0f);
		const float CrosshairAngleDeg = FMath::RadiansToDegrees(FMath::Acos(AimDot));
		const bool bDirectAimHit = SearchView.DirectAimHitActor.Get() == CandidateActor
			&& WorldDistanceCm <= Config.DirectSelectMaxDistanceCm;
		const bool bVisible = bDirectAimHit
			|| !SelectionContext.bRequireLineOfSightForNewSelection
			|| ContainsActor(SearchView.VisibleTargetActors, CandidateActor);
		const bool bProximityCandidate = bVisible
			&& WorldDistanceCm <= Config.ProximitySelectMaxDistanceCm
			&& CrosshairAngleDeg <= Config.ProximityHalfAngleDeg;

		if (!bDirectAimHit && !bProximityCandidate)
		{
			continue;
		}

		FCFTargetCandidate Candidate;
		Candidate.TargetActor = CandidateActor;
		Candidate.TargetWorldLocation = TargetWorldLocation;
		Candidate.DisplayInfo = ResolveCandidateTargetDisplayInfo(CandidateActor);
		Candidate.bDirectAimHit = bDirectAimHit;
		Candidate.bVisible = bVisible;
		Candidate.CrosshairAngleDeg = CrosshairAngleDeg;
		Candidate.NormalizedScreenDistance = CalculateNormalizedScreenDistance(SearchView, TargetWorldLocation);
		Candidate.WorldDistanceCm = WorldDistanceCm;
		Candidate.StableSortKey = Candidate.DisplayInfo.TargetId.IsNone() ? CandidateActor->GetFName() : Candidate.DisplayInfo.TargetId;
		SearchResult.SortedCandidates.Add(Candidate);

		if (bDirectAimHit)
		{
			++SearchResult.DirectAimCandidateCount;
		}
	}

	SearchResult.SortedCandidates.Sort(FCFTargetCandidateLess());
	SearchResult.AcceptedCandidateCount = SearchResult.SortedCandidates.Num();
	if (SearchResult.SortedCandidates.IsEmpty())
	{
		return SearchResult;
	}

			FCFTargetCandidate RawBestCandidate = *SearchResult.SortedCandidates.GetData();
	FCFTargetCandidate CurrentCandidate;
	if (ShouldKeepCurrentCandidateForStability(RawBestCandidate, SearchResult.SortedCandidates, Config, CurrentCandidate))
	{
		SearchResult.BestCandidate = CurrentCandidate;
		SearchResult.bKeptCurrentCandidateForStability = true;
	}
	else
	{
		SearchResult.BestCandidate = RawBestCandidate;
	}

	SearchResult.bHasBestCandidate = IsValid(SearchResult.BestCandidate.TargetActor.Get());
	return SearchResult;
}

bool UCFTargetSelectComp::ShouldKeepCurrentCandidateForStability(const FCFTargetCandidate& RawBestCandidate, const TArray<FCFTargetCandidate>& SortedCandidates, const FCFTargetSelectConfig& Config, FCFTargetCandidate& OutCurrentCandidate) const
{
	AActor* CurrentActor = GetCurrentCandidateActor();
	AActor* RawBestActor = RawBestCandidate.TargetActor.Get();
	if (!CurrentActor || !RawBestActor || CurrentActor == RawBestActor)
	{
		return false;
	}

	const FCFTargetCandidate* CurrentCandidate = nullptr;
	for (const FCFTargetCandidate& Candidate : SortedCandidates)
	{
		if (Candidate.TargetActor.Get() == CurrentActor)
		{
			CurrentCandidate = &Candidate;
			break;
		}
	}

	if (!CurrentCandidate)
	{
		return false;
	}

	if (RawBestCandidate.bDirectAimHit && !CurrentCandidate->bDirectAimHit)
	{
		return false;
	}

	if (CurrentCandidate->bDirectAimHit && !RawBestCandidate.bDirectAimHit)
	{
		OutCurrentCandidate = *CurrentCandidate;
		return true;
	}

	if (RawBestCandidate.bDirectAimHit || CurrentCandidate->bDirectAimHit)
	{
		return false;
	}

	const float SafeAdvantageRatio = FMath::Clamp(Config.CandidateSwitchAdvantageRatio, 0.0f, 1.0f);
	const float RequiredNewScreenDistance = CurrentCandidate->NormalizedScreenDistance * (1.0f - SafeAdvantageRatio);
	if (RawBestCandidate.NormalizedScreenDistance > RequiredNewScreenDistance + CandidateFloatTolerance)
	{
		OutCurrentCandidate = *CurrentCandidate;
		return true;
	}

	return false;
}

bool UCFTargetSelectComp::BuildRuntimeSearchView(FCFTargetSearchView& OutSearchView, TArray<AActor*>& OutCandidateActors) const
{
	ACFVehiclePawn* OwnerVehiclePawn = Cast<ACFVehiclePawn>(GetOwner());
	UWorld* CurrentWorld = GetWorld();
	UCFVehicleCameraComp* CameraComponent = OwnerVehiclePawn ? OwnerVehiclePawn->GetVehicleCameraComp() : nullptr;
	if (!OwnerVehiclePawn || !CurrentWorld || !CameraComponent)
	{
		return false;
	}

	OutSearchView.ViewOrigin = CameraComponent->GetCurrentAimTraceStartLocation();
	OutSearchView.ViewDirection = CameraComponent->GetCurrentAimDirection().GetSafeNormal();
	OutSearchView.ViewUpDirection = CameraComponent->FollowCamera
		? CameraComponent->FollowCamera->GetUpVector()
		: FVector::UpVector;

	const FCFVehicleCameraRuntimeState CameraState = CameraComponent->GetCameraRuntimeState();
	OutSearchView.VerticalFOVDeg = CameraState.CurrentFOV > KINDA_SMALL_NUMBER
		? CameraState.CurrentFOV
		: CameraComponent->FollowCamera
			? CameraComponent->FollowCamera->FieldOfView
			: 60.0f;

	OutSearchView.ViewportAspectRatio = 1.7777778f;
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		const FIntPoint ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		if (ViewportSize.Y > 0)
		{
			OutSearchView.ViewportAspectRatio = static_cast<float>(ViewportSize.X) / static_cast<float>(ViewportSize.Y);
		}
	}

	const FCFTargetSelectConfig Config = GetResolvedTargetSelectConfig();
	const FVector DirectTraceEnd = OutSearchView.ViewOrigin + OutSearchView.ViewDirection * Config.DirectSelectMaxDistanceCm;
	FHitResult DirectHitResult;
	FCollisionQueryParams DirectQueryParams(SCENE_QUERY_STAT(CFTargetSelectDirectTrace), false, OwnerVehiclePawn);
	if (CurrentWorld->LineTraceSingleByChannel(DirectHitResult, OutSearchView.ViewOrigin, DirectTraceEnd, CFCollisionChannels::TargetSelect, DirectQueryParams))
	{
		OutSearchView.DirectAimHitActor = DirectHitResult.GetActor();
	}

	for (TActorIterator<AActor> ActorIterator(CurrentWorld); ActorIterator; ++ActorIterator)
	{
		AActor* CandidateActor = *ActorIterator;
		if (!CanUseActorAsTarget(CandidateActor, DefaultSelectionContext))
		{
			continue;
		}

		OutCandidateActors.Add(CandidateActor);

		bool bVisible = !DefaultSelectionContext.bRequireLineOfSightForNewSelection
			|| OutSearchView.DirectAimHitActor.Get() == CandidateActor;
		if (!bVisible)
		{
			const FVector TargetLocation = ResolveTargetSelectionLocation(CandidateActor);
			FHitResult VisibilityHitResult;
			FCollisionQueryParams VisibilityQueryParams(SCENE_QUERY_STAT(CFTargetSelectVisibilityTrace), false, OwnerVehiclePawn);
			const bool bVisibilityBlocked = CurrentWorld->LineTraceSingleByChannel(VisibilityHitResult, OutSearchView.ViewOrigin, TargetLocation, CFCollisionChannels::TargetSelect, VisibilityQueryParams);
			bVisible = !bVisibilityBlocked || VisibilityHitResult.GetActor() == CandidateActor;
		}

		if (bVisible)
		{
			OutSearchView.VisibleTargetActors.Add(CandidateActor);
		}
	}

	return IsFiniteTargetCandidateVector(OutSearchView.ViewOrigin) && !OutSearchView.ViewDirection.IsNearlyZero();
}

bool UCFTargetSelectComp::RefreshCurrentCandidate()
{
	FCFTargetSearchView SearchView;
	TArray<AActor*> CandidateActors;
	if (!BuildRuntimeSearchView(SearchView, CandidateActors))
	{
		CacheLastCandidateSearchResult(FCFTargetSearchResult());
		ClearCurrentCandidate();
		return false;
	}

	const FCFTargetSearchResult SearchResult = EvaluateCandidateActors(CandidateActors, SearchView, DefaultSelectionContext);
	CacheLastCandidateSearchResult(SearchResult);

	if (!SearchResult.bHasBestCandidate)
	{
		ClearCurrentCandidate();
		return false;
	}

	return SetCurrentCandidate(SearchResult.BestCandidate, DefaultSelectionContext);
}

void UCFTargetSelectComp::CacheLastCandidateSearchResult(const FCFTargetSearchResult& SearchResult)
{
	LastCandidateSearchResult = SearchResult;
	LastCandidateActors.Reset();
	LastBestCandidateActor = SearchResult.BestCandidate.TargetActor.Get();
	LastCandidateSearchResult.BestCandidate.TargetActor = nullptr;

	for (FCFTargetCandidate& Candidate : LastCandidateSearchResult.SortedCandidates)
	{
		LastCandidateActors.Add(Candidate.TargetActor.Get());
		Candidate.TargetActor = nullptr;
	}
}

FCFTargetSearchResult UCFTargetSelectComp::GetLastCandidateSearchResult() const
{
	FCFTargetSearchResult ResultCopy = LastCandidateSearchResult;
	ResultCopy.BestCandidate.TargetActor = LastBestCandidateActor.Get();

	auto WeakActorIterator = LastCandidateActors.CreateConstIterator();
	for (FCFTargetCandidate& Candidate : ResultCopy.SortedCandidates)
	{
		if (WeakActorIterator)
		{
			Candidate.TargetActor = WeakActorIterator->Get();
			++WeakActorIterator;
		}
	}

	ResultCopy.bHasBestCandidate = ResultCopy.bHasBestCandidate && IsValid(ResultCopy.BestCandidate.TargetActor.Get());
	return ResultCopy;
}

FString UCFTargetSelectComp::BuildCandidateSearchDebugSummary() const
{
	const FCFTargetSearchResult SearchResult = GetLastCandidateSearchResult();
	const FString BestCandidateName = SearchResult.bHasBestCandidate && SearchResult.BestCandidate.TargetActor
		? SearchResult.BestCandidate.TargetActor->GetName()
		: TEXT("None");

	return FString::Printf(
		TEXT("TargetCandidateSearch: Input=%d, Accepted=%d, Direct=%d, Best=%s, KeptForStability=%s"),
		SearchResult.InputActorCount,
		SearchResult.AcceptedCandidateCount,
		SearchResult.DirectAimCandidateCount,
		*BestCandidateName,
		SearchResult.bKeptCurrentCandidateForStability ? TEXT("True") : TEXT("False"));
}

void UCFTargetSelectComp::DrawCandidateSearchDebug(const float DurationSeconds, const float SphereRadius) const
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		return;
	}

	const FCFTargetSearchResult SearchResult = GetLastCandidateSearchResult();
	const float SafeDuration = FMath::Max(0.0f, DurationSeconds);
	const float SafeRadius = FMath::Max(1.0f, SphereRadius);
	for (const FCFTargetCandidate& Candidate : SearchResult.SortedCandidates)
	{
		const bool bBestCandidate = SearchResult.bHasBestCandidate
			&& Candidate.TargetActor.Get() == SearchResult.BestCandidate.TargetActor.Get();
		const FColor CandidateColor = bBestCandidate
			? FColor::Cyan
			: Candidate.bDirectAimHit
				? FColor::Green
				: FColor::Yellow;
		DrawDebugSphere(CurrentWorld, Candidate.TargetWorldLocation, SafeRadius, 12, CandidateColor, false, SafeDuration, 0, 2.0f);
	}
}
