// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-08-15
// Description: TS-P0-03 후보 탐색·TS-P0-04 선택 수명과 TS-P0-08 지속형 디버그·LOS 사전필터·검색 진단 구현
// Changelog:
// - v1.5.0: 기존 Evaluate가 반드시 거부할 비-Direct 거리·반각 밖 Targetable Actor의 LOS Trace를 사전 생략하고 생략 수를 진단.
// - v1.4.0: 실제 런타임 후보 갱신의 월드 Actor 스캔 수, Visibility/전체 Trace 수와 View·수집·평가 전체 경과시간을 진단 결과에 기록.
// - v1.3.0: 자동 후보 디버그 Sphere를 한 프레임이 아니라 CandidateRefreshIntervalSec 동안 유지하고 반경을 컴포넌트 Debug 설정에서 읽도록 변경.
// - v1.2.1: 기존 v1.2.0 의도와 달리 남아 있던 익명 네임스페이스 IsFiniteVector를 IsFiniteTargetCandidateVector로 실제 교정해 Unity Build 충돌을 제거.
// - v1.2.0: Unity 빌드에서 다른 구현 파일의 익명 네임스페이스 심볼과 충돌하지 않도록 후보 탐색 전용 이름으로 분리.
// Migration:
// - v1.5.0 사전필터는 LOS Trace 수만 줄이며 모든 Targetable Actor를 OutCandidateActors에 유지한다. 직접 조준 후보와 기존 Evaluate/정렬/히스테리시스 계약은 변경하지 않는다.
// - TS-P0-08 디버그 표시 변경은 시각 진단 수명·반경만 다루며 ProximityHalfAngleDeg, 거리, 히스테리시스와 후보 정렬 계약은 변경하지 않는다.
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
#include "HAL/PlatformTime.h"
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
		// [v1.3.0] 한 프레임 표시가 후보 갱신 사이에서 사라지지 않도록 다음 예정 갱신까지 Debug Sphere를 유지합니다.
		DrawCandidateSearchDebug(RefreshIntervalSeconds, CandidateSearchDebugSphereRadiusCm);
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

bool UCFTargetSelectComp::BuildRuntimeSearchView(
	FCFTargetSearchView& OutSearchView,
	TArray<AActor*>& OutCandidateActors,
			int32& OutWorldActorScanCount,
	int32& OutVisibilityTraceCount,
	int32& OutVisibilityPrefilterSkipCount,
	int32& OutTotalTraceCount) const
{
	// [v1.5.0] 호출자가 실패 경로에서도 이전 프레임 수치를 재사용하지 않도록 런타임 진단값을 먼저 초기화합니다.
	OutWorldActorScanCount = 0;
	OutVisibilityTraceCount = 0;
	OutVisibilityPrefilterSkipCount = 0;
	OutTotalTraceCount = 0;

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
	// [v1.4.0] 런타임 후보 갱신은 카메라 중앙 직접 조준 Trace를 정확히 한 번 수행합니다.
	++OutTotalTraceCount;
	if (CurrentWorld->LineTraceSingleByChannel(DirectHitResult, OutSearchView.ViewOrigin, DirectTraceEnd, CFCollisionChannels::TargetSelect, DirectQueryParams))
	{
		OutSearchView.DirectAimHitActor = DirectHitResult.GetActor();
	}

				for (TActorIterator<AActor> ActorIterator(CurrentWorld); ActorIterator; ++ActorIterator)
	{
		// [v1.4.0] 현재 구현이 매 후보 갱신마다 실제로 순회한 전체 월드 Actor 수를 성능 진단에 기록합니다.
		++OutWorldActorScanCount;
		AActor* CandidateActor = *ActorIterator;
		if (!CanUseActorAsTarget(CandidateActor, DefaultSelectionContext))
		{
			continue;
		}

						OutCandidateActors.Add(CandidateActor);

		// [v1.5.0] 직접 조준 적중 Actor는 기존처럼 별도 LOS Trace 없이 visible로 취급합니다.
		const bool bDirectAimActor = OutSearchView.DirectAimHitActor.Get() == CandidateActor;
		bool bVisible = !DefaultSelectionContext.bRequireLineOfSightForNewSelection || bDirectAimActor;
		if (!bVisible)
		{
			// [v1.5.0] 기존 EvaluateCandidateActors와 동일한 TargetPoint 위치를 사용해 LOS 전에 proximity 가능성만 판정합니다.
			const FVector TargetLocation = ResolveTargetSelectionLocation(CandidateActor);
			const FVector ToTarget = TargetLocation - OutSearchView.ViewOrigin;
			const float WorldDistanceCm = ToTarget.Size();
			const FVector DirectionToTarget = ToTarget.GetSafeNormal();
			const float AimDot = DirectionToTarget.IsNearlyZero()
				? 0.0f
				: FMath::Clamp(FVector::DotProduct(OutSearchView.ViewDirection, DirectionToTarget), -1.0f, 1.0f);
			const float CrosshairAngleDeg = FMath::RadiansToDegrees(FMath::Acos(AimDot));
			const bool bCanBecomeProximityCandidate = IsFiniteTargetCandidateVector(TargetLocation)
				&& FMath::IsFinite(WorldDistanceCm)
				&& FMath::IsFinite(CrosshairAngleDeg)
				&& WorldDistanceCm <= Config.ProximitySelectMaxDistanceCm
				&& CrosshairAngleDeg <= Config.ProximityHalfAngleDeg;
			if (!bCanBecomeProximityCandidate)
			{
				// [v1.5.0] 후보 배열은 유지하지만 기존 Evaluate가 반드시 거부할 대상이므로 LOS Trace만 생략합니다.
				++OutVisibilityPrefilterSkipCount;
				continue;
			}

			FHitResult VisibilityHitResult;
			FCollisionQueryParams VisibilityQueryParams(SCENE_QUERY_STAT(CFTargetSelectVisibilityTrace), false, OwnerVehiclePawn);
			// [v1.5.0] 실제 proximity 후보 가능성이 남은 대상에만 LOS Trace를 수행합니다.
			++OutVisibilityTraceCount;
			++OutTotalTraceCount;
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
	// [v1.4.0] View 생성·월드 Actor 수집·Trace·후보 평가 전체의 실제 검색 시간을 측정할 시작 시각입니다.
	const double SearchStartSeconds = FPlatformTime::Seconds();
	FCFTargetSearchView SearchView;
	TArray<AActor*> CandidateActors;
	// [v1.4.0] 이번 런타임 검색이 TActorIterator로 순회한 전체 Actor 수입니다.
	int32 WorldActorScanCount = 0;
			// [v1.4.0] 이번 런타임 검색이 대상 시야 확인에 실제 사용한 Visibility Trace 수입니다.
	int32 VisibilityTraceCount = 0;
	// [v1.5.0] 기존 proximity 거리·반각 밖이라 LOS Trace를 생략한 Targetable Actor 수입니다.
	int32 VisibilityPrefilterSkipCount = 0;
	// [v1.4.0] 직접 조준 Trace와 Visibility Trace를 합친 전체 Trace 수입니다.
	int32 TotalTraceCount = 0;
	if (!BuildRuntimeSearchView(SearchView, CandidateActors, WorldActorScanCount, VisibilityTraceCount, VisibilityPrefilterSkipCount, TotalTraceCount))
	{
		// [v1.4.0] 검색 View 구성 실패도 비용과 실제 시도 횟수를 잃지 않고 마지막 진단 결과로 남깁니다.
		FCFTargetSearchResult FailedSearchResult;
						FailedSearchResult.RuntimeWorldActorScanCount = WorldActorScanCount;
		FailedSearchResult.RuntimeVisibilityTraceCount = VisibilityTraceCount;
		FailedSearchResult.RuntimeVisibilityPrefilterSkipCount = VisibilityPrefilterSkipCount;
		FailedSearchResult.RuntimeTotalTraceCount = TotalTraceCount;
		FailedSearchResult.RuntimeSearchDurationMs = static_cast<float>((FPlatformTime::Seconds() - SearchStartSeconds) * 1000.0);
		CacheLastCandidateSearchResult(FailedSearchResult);
		ClearCurrentCandidate();
		return false;
	}

	// [v1.4.0] 기존 결정적 후보 판정을 그대로 수행한 뒤 런타임 성능 진단값만 결과에 덧붙입니다.
	FCFTargetSearchResult SearchResult = EvaluateCandidateActors(CandidateActors, SearchView, DefaultSelectionContext);
		SearchResult.RuntimeWorldActorScanCount = WorldActorScanCount;
	SearchResult.RuntimeVisibilityTraceCount = VisibilityTraceCount;
	SearchResult.RuntimeVisibilityPrefilterSkipCount = VisibilityPrefilterSkipCount;
	SearchResult.RuntimeTotalTraceCount = TotalTraceCount;
	SearchResult.RuntimeSearchDurationMs = static_cast<float>((FPlatformTime::Seconds() - SearchStartSeconds) * 1000.0);
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
				TEXT("TargetCandidateSearch: Input=%d, Accepted=%d, Direct=%d, WorldScanned=%d, VisibilityTraces=%d, PrefilterSkipped=%d, TotalTraces=%d, SearchMs=%.4f, Best=%s, KeptForStability=%s"),
		SearchResult.InputActorCount,
		SearchResult.AcceptedCandidateCount,
		SearchResult.DirectAimCandidateCount,
		SearchResult.RuntimeWorldActorScanCount,
		SearchResult.RuntimeVisibilityTraceCount,
		SearchResult.RuntimeVisibilityPrefilterSkipCount,
		SearchResult.RuntimeTotalTraceCount,
		SearchResult.RuntimeSearchDurationMs,
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
