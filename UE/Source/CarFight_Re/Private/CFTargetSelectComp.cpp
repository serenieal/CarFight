// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-07-24
// Description: CarFight 타겟 선택 상태, 선택 수명과 장비 조회 컴포넌트 구현
// Scope: 설정·필터·후보·선택 수명과 장비별 읽기 전용 사용 가능 평가를 제공합니다.
// Changelog:
// - v1.3.0: TS-P0-07 장비 요청의 준비·호환·추적·거리 조건과 구체적인 실패 사유 평가를 구현.
// - v1.2.1: OnDestroyed를 추가 구독해 BeginPlay 이전 Actor의 Destroy도 즉시 선택 해제하도록 보강.
// - v1.2.0: 선택 대상 VehicleHealth·EndPlay 수명 구독, 가림 유예, 거리 이탈과 비활성화 해제를 구현.
// - v1.0.1: Native C++ TargetSelectable은 직접 구현을 호출하고 실제 Blueprint 재정의만 Execute 경로로 전달하는 안전 디스패치를 추가.
// - v1.0.0: TS-P0-01 후보·선택 상태 저장, 약한 참조, 안전 getter와 Blueprint 이벤트를 구현.
// Migration:
// - 선택 대상 변경 또는 해제 시 이전 대상의 OnDestroyed, OnEndPlay와 VehicleHealth 파괴 이벤트 구독을 반드시 제거한다.
// - 화면 밖 이동과 카메라 방향 변경은 선택 해제 조건이 아니며, 월드 시야가 OcclusionGracePeriodSec 이상 끊긴 경우에만 P0 선택을 해제한다.
// - 선택 대상 무효화 또는 해제 뒤 현재 후보를 자동 선택하지 않는다.

#include "CFTargetSelectComp.h"

#include "CFCollisionChannels.h"
#include "CFTargetSelectData.h"
#include "CFTargetSelectable.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	const FName IsTargetSelectableFunctionName(TEXT("IsTargetSelectable"));
	const FName GetTargetDisplayInfoFunctionName(TEXT("GetTargetDisplayInfo"));
	const FName GetTargetSelectionLocationFunctionName(TEXT("GetTargetSelectionLocation"));
	const FName GetTargetTrackStateFunctionName(TEXT("GetTargetTrackState"));

	bool HasBlueprintTargetSelectableOverride(const AActor* TargetActor, const FName FunctionName)
	{
		if (!IsValid(TargetActor))
		{
			return false;
		}

		const UFunction* TargetFunction = TargetActor->FindFunction(FunctionName);
		return IsValid(TargetFunction)
			&& TargetFunction->GetOuter() != UCFTargetSelectable::StaticClass();
	}

	bool ResolveIsTargetSelectable(const AActor* TargetActor, const FCFTargetSelectionContext& SelectionContext)
	{
		if (HasBlueprintTargetSelectableOverride(TargetActor, IsTargetSelectableFunctionName))
		{
			return ICFTargetSelectable::Execute_IsTargetSelectable(TargetActor, SelectionContext);
		}

		if (const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor))
		{
			return NativeTargetSelectable->IsTargetSelectable_Implementation(SelectionContext);
		}

		return ICFTargetSelectable::Execute_IsTargetSelectable(TargetActor, SelectionContext);
	}

	FCFTargetDisplayInfo ResolveTargetDisplayInfo(const AActor* TargetActor)
	{
		if (HasBlueprintTargetSelectableOverride(TargetActor, GetTargetDisplayInfoFunctionName))
		{
			return ICFTargetSelectable::Execute_GetTargetDisplayInfo(TargetActor);
		}

		if (const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor))
		{
			return NativeTargetSelectable->GetTargetDisplayInfo_Implementation();
		}

		return ICFTargetSelectable::Execute_GetTargetDisplayInfo(TargetActor);
	}

	ECFTargetTrackState ResolveTargetTrackState(const AActor* TargetActor)
	{
		if (HasBlueprintTargetSelectableOverride(TargetActor, GetTargetTrackStateFunctionName))
		{
			return ICFTargetSelectable::Execute_GetTargetTrackState(TargetActor);
		}

		if (const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor))
		{
			return NativeTargetSelectable->GetTargetTrackState_Implementation();
		}

		return ICFTargetSelectable::Execute_GetTargetTrackState(TargetActor);
	}
}

UCFTargetSelectComp::UCFTargetSelectComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(false);
}

void UCFTargetSelectComp::Deactivate()
{
	if (bHasSelectedTarget)
	{
		NotifySelectedTargetValidityChanged(SelectedTargetActor.Get(), false);
		ClearSelectedTarget(ECFTargetClearReason::SystemDisabled);
	}

	ClearCurrentCandidate();
	Super::Deactivate();
}

void UCFTargetSelectComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bHasSelectedTarget)
	{
		NotifySelectedTargetValidityChanged(SelectedTargetActor.Get(), false);
		ClearSelectedTarget(ECFTargetClearReason::OwnerDestroyed);
	}
	else
	{
		UnbindSelectedTargetLifetimeEvents();
	}

	ClearCurrentCandidate();
	Super::EndPlay(EndPlayReason);
}

FCFTargetSelectConfig UCFTargetSelectComp::GetResolvedTargetSelectConfig() const
{
	if (IsValid(TargetSelectData) && TargetSelectData->IsTargetSelectConfigValid())
	{
		return TargetSelectData->TargetSelectConfig;
	}

	return FallbackTargetSelectConfig;
}

bool UCFTargetSelectComp::CanUseActorAsTarget(AActor* TargetActor, const FCFTargetSelectionContext& SelectionContext) const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	const AActor* OwnerActor = GetOwner();
	if (!SelectionContext.bAllowSelfTarget && OwnerActor && TargetActor == OwnerActor)
	{
		return false;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UCFTargetSelectable::StaticClass()))
	{
		return false;
	}

	if (const UCFVehicleHealthComp* VehicleHealthComponent = TargetActor->FindComponentByClass<UCFVehicleHealthComp>())
	{
		if (VehicleHealthComponent->IsDestroyed())
		{
			return false;
		}
	}

	if (!ResolveIsTargetSelectable(TargetActor, SelectionContext))
	{
		return false;
	}

	return DoesDisplayInfoMatchContext(ResolveTargetDisplayInfo(TargetActor), SelectionContext);
}

bool UCFTargetSelectComp::SetCurrentCandidate(const FCFTargetCandidate& Candidate, const FCFTargetSelectionContext& SelectionContext)
{
	AActor* NewCandidateActor = Candidate.TargetActor.Get();
	if (!CanUseActorAsTarget(NewCandidateActor, SelectionContext))
	{
		return false;
	}

	AActor* PreviousCandidateActor = GetCurrentCandidateActor();
	const bool bSameCandidateActor = bHasCurrentCandidate && CurrentCandidateActor.Get() == NewCandidateActor;

	CurrentCandidateActor = NewCandidateActor;
	CurrentCandidateData = Candidate;
	CurrentCandidateData.TargetActor = nullptr;
	bHasCurrentCandidate = true;

	if (bSameCandidateActor)
	{
		return true;
	}

	FCFTargetCandidate EventCandidateData = CurrentCandidateData;
	EventCandidateData.TargetActor = NewCandidateActor;
	OnTargetCandidateChanged.Broadcast(PreviousCandidateActor, NewCandidateActor, EventCandidateData);
	return true;
}

void UCFTargetSelectComp::ClearCurrentCandidate()
{
	if (!bHasCurrentCandidate)
	{
		return;
	}

	AActor* PreviousCandidateActor = GetCurrentCandidateActor();
	CurrentCandidateActor.Reset();
	CurrentCandidateData = FCFTargetCandidate();
	bHasCurrentCandidate = false;
	OnTargetCandidateChanged.Broadcast(PreviousCandidateActor, nullptr, FCFTargetCandidate());
}

bool UCFTargetSelectComp::SetSelectedTarget(AActor* TargetActor, const FCFTargetSelectionContext& SelectionContext)
{
	if (!CanUseActorAsTarget(TargetActor, SelectionContext))
	{
		return false;
	}

	if (bHasSelectedTarget && SelectedTargetActor.Get() == TargetActor)
	{
		return true;
	}

	AActor* PreviousTargetActor = GetSelectedTargetActor();
	UnbindSelectedTargetLifetimeEvents();

	SelectedTargetActor = TargetActor;
	SelectedTargetDisplayInfo = ResolveTargetDisplayInfo(TargetActor);
	SelectedTargetTrackState = ResolveTargetTrackState(TargetActor);
	SelectedTargetOcclusionElapsedSeconds = 0.0f;
	SelectedTargetLifetimeCheckElapsedSeconds = 0.0f;
	bSelectedTargetValid = true;
	bHasSelectedTarget = true;
	BindSelectedTargetLifetimeEvents(TargetActor);

	OnSelectedTargetChanged.Broadcast(PreviousTargetActor, TargetActor, SelectedTargetDisplayInfo);
	return true;
}

void UCFTargetSelectComp::ClearSelectedTarget(const ECFTargetClearReason ClearReason)
{
	if (!bHasSelectedTarget)
	{
		return;
	}

	AActor* ClearedTargetActor = SelectedTargetActor.Get();
	if (!ClearedTargetActor)
	{
		ClearedTargetActor = BoundSelectedTargetActor.Get();
	}

	UnbindSelectedTargetLifetimeEvents();
	SelectedTargetActor.Reset();
	SelectedTargetDisplayInfo = FCFTargetDisplayInfo();
	SelectedTargetTrackState = ECFTargetTrackState::Invalid;
	SelectedTargetOcclusionElapsedSeconds = 0.0f;
	SelectedTargetLifetimeCheckElapsedSeconds = 0.0f;
	LastSelectedTargetClearReason = ClearReason;
	bSelectedTargetValid = false;
	bHasSelectedTarget = false;
	OnSelectedTargetCleared.Broadcast(ClearedTargetActor, ClearReason);
}

bool UCFTargetSelectComp::NotifySelectedTargetValidityChanged(AActor* TargetActor, const bool bIsValidTarget)
{
	if (!bHasSelectedTarget)
	{
		return false;
	}

	AActor* CurrentSelectedActor = SelectedTargetActor.Get();
	AActor* BoundTargetActor = BoundSelectedTargetActor.Get();
	if (TargetActor && CurrentSelectedActor != TargetActor && BoundTargetActor != TargetActor)
	{
		return false;
	}

	AActor* EventTargetActor = CurrentSelectedActor ? CurrentSelectedActor : TargetActor;
	const bool bResolvedValidity = bIsValidTarget && IsValid(CurrentSelectedActor);
	if (bSelectedTargetValid == bResolvedValidity)
	{
		return true;
	}

	bSelectedTargetValid = bResolvedValidity;
	OnSelectedTargetValidityChanged.Broadcast(EventTargetActor, bSelectedTargetValid);
	return true;
}

FVector UCFTargetSelectComp::ResolveTargetSelectionLocation(AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return FVector::ZeroVector;
	}

	if (HasBlueprintTargetSelectableOverride(TargetActor, GetTargetSelectionLocationFunctionName))
	{
		return ICFTargetSelectable::Execute_GetTargetSelectionLocation(TargetActor);
	}

	if (const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor))
	{
		return NativeTargetSelectable->GetTargetSelectionLocation_Implementation();
	}

	return ICFTargetSelectable::Execute_GetTargetSelectionLocation(TargetActor);
}

void UCFTargetSelectComp::BindSelectedTargetLifetimeEvents(AActor* TargetActor)
{
	UnbindSelectedTargetLifetimeEvents();
	if (!IsValid(TargetActor))
	{
		return;
	}

			BoundSelectedTargetActor = TargetActor;
	TargetActor->OnDestroyed.AddUniqueDynamic(this, &UCFTargetSelectComp::HandleSelectedTargetDestroyed);
	TargetActor->OnEndPlay.AddUniqueDynamic(this, &UCFTargetSelectComp::HandleSelectedTargetEndPlay);

	if (UCFVehicleHealthComp* VehicleHealthComponent = TargetActor->FindComponentByClass<UCFVehicleHealthComp>())
	{
		BoundSelectedVehicleHealthComp = VehicleHealthComponent;
		VehicleHealthComponent->OnVehicleDestroyed.AddUniqueDynamic(this, &UCFTargetSelectComp::HandleSelectedVehicleDestroyed);
	}
}

void UCFTargetSelectComp::UnbindSelectedTargetLifetimeEvents()
{
			if (AActor* BoundTargetActor = BoundSelectedTargetActor.Get())
	{
		BoundTargetActor->OnDestroyed.RemoveDynamic(this, &UCFTargetSelectComp::HandleSelectedTargetDestroyed);
		BoundTargetActor->OnEndPlay.RemoveDynamic(this, &UCFTargetSelectComp::HandleSelectedTargetEndPlay);
	}

	if (UCFVehicleHealthComp* VehicleHealthComponent = BoundSelectedVehicleHealthComp.Get())
	{
		VehicleHealthComponent->OnVehicleDestroyed.RemoveDynamic(this, &UCFTargetSelectComp::HandleSelectedVehicleDestroyed);
	}

	BoundSelectedTargetActor.Reset();
	BoundSelectedVehicleHealthComp.Reset();
}

void UCFTargetSelectComp::HandleSelectedTargetDestroyed(AActor* DestroyedActor)
{
	if (!bHasSelectedTarget || !DestroyedActor
		|| (SelectedTargetActor.Get() != DestroyedActor && BoundSelectedTargetActor.Get() != DestroyedActor))
	{
		return;
	}

	NotifySelectedTargetValidityChanged(DestroyedActor, false);
	ClearSelectedTarget(ECFTargetClearReason::Destroyed);
}

void UCFTargetSelectComp::HandleSelectedTargetEndPlay(AActor* EndedActor, const EEndPlayReason::Type EndPlayReason)
{
	if (!bHasSelectedTarget || !EndedActor
		|| (SelectedTargetActor.Get() != EndedActor && BoundSelectedTargetActor.Get() != EndedActor))
	{
		return;
	}

	NotifySelectedTargetValidityChanged(EndedActor, false);
	const ECFTargetClearReason ClearReason = EndPlayReason == EEndPlayReason::Destroyed
		? ECFTargetClearReason::Destroyed
		: ECFTargetClearReason::InvalidTarget;
	ClearSelectedTarget(ClearReason);
}

void UCFTargetSelectComp::HandleSelectedVehicleDestroyed(FCFDamageHitContext DamageHitContext)
{
	AActor* DestroyedTargetActor = DamageHitContext.HitActor;
	if (!DestroyedTargetActor)
	{
		DestroyedTargetActor = BoundSelectedTargetActor.Get();
	}

	if (!bHasSelectedTarget
		|| (SelectedTargetActor.Get() != DestroyedTargetActor && BoundSelectedTargetActor.Get() != DestroyedTargetActor))
	{
		return;
	}

	NotifySelectedTargetValidityChanged(DestroyedTargetActor, false);
	ClearSelectedTarget(ECFTargetClearReason::Destroyed);
}

bool UCFTargetSelectComp::IsSelectedTargetVisibleInWorld(AActor* TargetActor) const
{
	const AActor* OwnerActor = GetOwner();
	UWorld* CurrentWorld = GetWorld();
	if (!OwnerActor || !CurrentWorld || !IsValid(TargetActor))
	{
		return true;
	}

	FVector TraceStart = OwnerActor->GetActorLocation();
	if (const ACFVehiclePawn* OwnerVehiclePawn = Cast<ACFVehiclePawn>(OwnerActor))
	{
		if (const UCFVehicleCameraComp* CameraComponent = OwnerVehiclePawn->GetVehicleCameraComp())
		{
			TraceStart = CameraComponent->GetCurrentAimTraceStartLocation();
		}
	}

	const FVector TraceEnd = ResolveTargetSelectionLocation(TargetActor);
	FHitResult VisibilityHitResult;
	FCollisionQueryParams VisibilityQueryParams(SCENE_QUERY_STAT(CFTargetSelectedVisibilityTrace), false, OwnerActor);
	const bool bHasBlockingHit = CurrentWorld->LineTraceSingleByChannel(
		VisibilityHitResult,
		TraceStart,
		TraceEnd,
		CFCollisionChannels::TargetSelect,
		VisibilityQueryParams);

	return !bHasBlockingHit || VisibilityHitResult.GetActor() == TargetActor;
}

bool UCFTargetSelectComp::RefreshSelectedTargetLifetime(const float DeltaTime)
{
	if (!bHasSelectedTarget)
	{
		return false;
	}

	AActor* CurrentSelectedActor = SelectedTargetActor.Get();
	if (!IsValid(CurrentSelectedActor))
	{
		NotifySelectedTargetValidityChanged(nullptr, false);
		ClearSelectedTarget(ECFTargetClearReason::InvalidTarget);
		return false;
	}

	if (!CanUseActorAsTarget(CurrentSelectedActor, DefaultSelectionContext))
	{
		const UCFVehicleHealthComp* VehicleHealthComponent = CurrentSelectedActor->FindComponentByClass<UCFVehicleHealthComp>();
		const ECFTargetClearReason ClearReason = VehicleHealthComponent && VehicleHealthComponent->IsDestroyed()
			? ECFTargetClearReason::Destroyed
			: ECFTargetClearReason::InvalidTarget;
		NotifySelectedTargetValidityChanged(CurrentSelectedActor, false);
		ClearSelectedTarget(ClearReason);
		return false;
	}

	const FVector TargetLocation = ResolveTargetSelectionLocation(CurrentSelectedActor);
	const AActor* OwnerActor = GetOwner();
	const float MaxTrackingDistanceCm = FMath::Max(0.0f, GetResolvedTargetSelectConfig().DirectSelectMaxDistanceCm);
	if (OwnerActor && MaxTrackingDistanceCm > KINDA_SMALL_NUMBER
		&& FVector::Distance(OwnerActor->GetActorLocation(), TargetLocation) > MaxTrackingDistanceCm)
	{
		NotifySelectedTargetValidityChanged(CurrentSelectedActor, false);
		ClearSelectedTarget(ECFTargetClearReason::OutOfTrackingRange);
		return false;
	}

	return UpdateSelectedTargetVisibility(IsSelectedTargetVisibleInWorld(CurrentSelectedActor), DeltaTime);
}

bool UCFTargetSelectComp::UpdateSelectedTargetVisibility(const bool bIsVisible, const float DeltaTime)
{
	AActor* CurrentSelectedActor = GetSelectedTargetActor();
	if (!CurrentSelectedActor)
	{
		return false;
	}

	if (bIsVisible)
	{
		SelectedTargetOcclusionElapsedSeconds = 0.0f;
		SetSelectedTargetTrackState(CurrentSelectedActor, ECFTargetTrackState::Visible);
		return true;
	}

	SelectedTargetOcclusionElapsedSeconds += FMath::Max(0.0f, DeltaTime);
	SetSelectedTargetTrackState(CurrentSelectedActor, ECFTargetTrackState::Occluded);

	const float OcclusionGracePeriodSeconds = FMath::Max(0.0f, GetResolvedTargetSelectConfig().OcclusionGracePeriodSec);
	if (SelectedTargetOcclusionElapsedSeconds + KINDA_SMALL_NUMBER < OcclusionGracePeriodSeconds)
	{
		return true;
	}

	NotifySelectedTargetValidityChanged(CurrentSelectedActor, false);
	ClearSelectedTarget(ECFTargetClearReason::InvalidTarget);
	return false;
}

FString UCFTargetSelectComp::BuildSelectedTargetLifetimeDebugSummary() const
{
	AActor* CurrentSelectedActor = SelectedTargetActor.Get();
	return FString::Printf(
		TEXT("SelectedTargetLifetime: Target=%s, HasRecord=%s, Valid=%s, Track=%s, Occluded=%.2fs, LastClear=%s"),
		CurrentSelectedActor ? *CurrentSelectedActor->GetName() : TEXT("None"),
		bHasSelectedTarget ? TEXT("True") : TEXT("False"),
		IsSelectedTargetValid() ? TEXT("True") : TEXT("False"),
		*UEnum::GetValueAsString(SelectedTargetTrackState),
		SelectedTargetOcclusionElapsedSeconds,
		*UEnum::GetValueAsString(LastSelectedTargetClearReason));
}

bool UCFTargetSelectComp::SetSelectedTargetTrackState(AActor* TargetActor, const ECFTargetTrackState NewTrackState)
{
	if (!bHasSelectedTarget)
	{
		return false;
	}

	AActor* CurrentSelectedActor = SelectedTargetActor.Get();
	if (TargetActor && CurrentSelectedActor != TargetActor)
	{
		return false;
	}

	if (SelectedTargetTrackState == NewTrackState)
	{
		return true;
	}

	const ECFTargetTrackState PreviousTrackState = SelectedTargetTrackState;
	SelectedTargetTrackState = NewTrackState;
	OnSelectedTargetTrackStateChanged.Broadcast(CurrentSelectedActor, PreviousTrackState, SelectedTargetTrackState);
	return true;
}

AActor* UCFTargetSelectComp::GetCurrentCandidateActor() const
{
	AActor* CandidateActor = CurrentCandidateActor.Get();
	return bHasCurrentCandidate && IsValid(CandidateActor) ? CandidateActor : nullptr;
}

FCFTargetCandidate UCFTargetSelectComp::GetCurrentCandidateData() const
{
	AActor* CandidateActor = GetCurrentCandidateActor();
	if (!CandidateActor)
	{
		return FCFTargetCandidate();
	}

	FCFTargetCandidate CandidateDataCopy = CurrentCandidateData;
	CandidateDataCopy.TargetActor = CandidateActor;
	return CandidateDataCopy;
}

bool UCFTargetSelectComp::HasCurrentCandidate() const
{
	return GetCurrentCandidateActor() != nullptr;
}

AActor* UCFTargetSelectComp::GetSelectedTargetActor() const
{
	AActor* TargetActor = SelectedTargetActor.Get();
	return bHasSelectedTarget && IsValid(TargetActor) ? TargetActor : nullptr;
}

FCFTargetDisplayInfo UCFTargetSelectComp::GetSelectedTargetDisplayInfo() const
{
	return bHasSelectedTarget ? SelectedTargetDisplayInfo : FCFTargetDisplayInfo();
}

bool UCFTargetSelectComp::IsSelectedTargetValid() const
{
	return bHasSelectedTarget && bSelectedTargetValid && IsValid(SelectedTargetActor.Get());
}

ECFTargetCoreState UCFTargetSelectComp::GetTargetCoreState() const
{
	if (bHasSelectedTarget)
	{
		return IsSelectedTargetValid()
			? ECFTargetCoreState::TargetSelected
			: ECFTargetCoreState::SelectedTargetInvalid;
	}

	return HasCurrentCandidate()
		? ECFTargetCoreState::CandidateAvailable
		: ECFTargetCoreState::NoCandidate;
}

ECFTargetTrackState UCFTargetSelectComp::GetSelectedTargetTrackState() const
{
	return IsSelectedTargetValid() ? SelectedTargetTrackState : ECFTargetTrackState::Invalid;
}

FCFTargetUseResult UCFTargetSelectComp::EvaluateSelectedTargetForUse(const FCFTargetUseRequest& UseRequest) const
{
	FCFTargetUseResult UseResult;
	UseResult.EquipmentId = UseRequest.EquipmentId;
	UseResult.bEquipmentReady = UseRequest.bEquipmentReady;
	UseResult.MaxUseDistanceCm = FMath::Max(UseRequest.MaxUseDistanceCm, 0.0f);

	auto SetFailure = [&UseResult](const ECFTargetUseFailureReason FailureReason, const FString& ResultMessage, const FName FailureAttributeTag = NAME_None)
	{
		UseResult.bCanUseTarget = false;
		UseResult.FailureReason = FailureReason;
		UseResult.FailureAttributeTag = FailureAttributeTag;
		UseResult.ResultMessage = FText::FromString(ResultMessage);
	};

			UseResult.bHasSelectedTarget = HasSelectedTarget();
	UseResult.TargetActor = GetSelectedTargetActor();
	UseResult.bSelectedTargetValid = IsSelectedTargetValid();

	if (!UseRequest.bEquipmentReady)
	{
		SetFailure(ECFTargetUseFailureReason::EquipmentUnavailable, TEXT("장비가 현재 타겟 사용 요청을 처리할 준비가 되지 않았습니다."));
		return UseResult;
	}

	if (!UseResult.bHasSelectedTarget)
	{
		SetFailure(ECFTargetUseFailureReason::NoSelectedTarget, TEXT("현재 선택된 타겟이 없습니다."));
		return UseResult;
	}

		if (!UseResult.bSelectedTargetValid || !IsValid(UseResult.TargetActor))
	{
		SetFailure(ECFTargetUseFailureReason::SelectedTargetInvalid, TEXT("선택 기록은 있지만 대상이 더 이상 유효하지 않습니다."));
		return UseResult;
	}

		UseResult.DisplayInfo = ResolveTargetDisplayInfo(UseResult.TargetActor);
	UseResult.TrackState = GetSelectedTargetTrackState();

	const AActor* OwnerActor = GetOwner();
	const FVector UseOrigin = UseRequest.bUseExplicitOrigin
		? UseRequest.ExplicitUseOrigin
		: (OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector);
	UseResult.DistanceCm = FVector::Distance(UseOrigin, ResolveTargetSelectionLocation(UseResult.TargetActor));

	const FCFTargetUsePolicy& TargetPolicy = UseRequest.TargetPolicy;
	if (!TargetPolicy.AllowedCategories.IsEmpty()
		&& !TargetPolicy.AllowedCategories.Contains(UseResult.DisplayInfo.TargetCategory))
	{
		SetFailure(ECFTargetUseFailureReason::CategoryNotAllowed, TEXT("선택 대상의 분류가 이 장비와 호환되지 않습니다."));
		return UseResult;
	}

	if (!TargetPolicy.AllowedRelations.IsEmpty()
		&& !TargetPolicy.AllowedRelations.Contains(UseResult.DisplayInfo.Relation))
	{
		SetFailure(ECFTargetUseFailureReason::RelationNotAllowed, TEXT("선택 대상의 관계가 이 장비와 호환되지 않습니다."));
		return UseResult;
	}

	for (const FName RequiredAttributeTag : TargetPolicy.RequiredAttributeTags)
	{
		if (!UseResult.DisplayInfo.AttributeTags.Contains(RequiredAttributeTag))
		{
			SetFailure(
				ECFTargetUseFailureReason::MissingRequiredTag,
				FString::Printf(TEXT("장비 사용에 필요한 대상 속성 '%s'이 없습니다."), *RequiredAttributeTag.ToString()),
				RequiredAttributeTag);
			return UseResult;
		}
	}

	for (const FName ExcludedAttributeTag : TargetPolicy.ExcludedAttributeTags)
	{
		if (UseResult.DisplayInfo.AttributeTags.Contains(ExcludedAttributeTag))
		{
			SetFailure(
				ECFTargetUseFailureReason::ExcludedByTag,
				FString::Printf(TEXT("대상 속성 '%s' 때문에 이 장비를 사용할 수 없습니다."), *ExcludedAttributeTag.ToString()),
				ExcludedAttributeTag);
			return UseResult;
		}
	}

	const bool bTrackStateAllowed = TargetPolicy.AllowedTrackStates.IsEmpty()
		? UseResult.TrackState != ECFTargetTrackState::Invalid
		: TargetPolicy.AllowedTrackStates.Contains(UseResult.TrackState);
	if (!bTrackStateAllowed)
	{
		SetFailure(ECFTargetUseFailureReason::TrackStateNotAllowed, TEXT("현재 타겟 추적 상태에서는 이 장비를 사용할 수 없습니다."));
		return UseResult;
	}

	UseResult.bTargetCompatible = true;
	UseResult.bWithinUseDistance = UseResult.MaxUseDistanceCm <= KINDA_SMALL_NUMBER
		|| UseResult.DistanceCm <= UseResult.MaxUseDistanceCm + KINDA_SMALL_NUMBER;
	if (!UseResult.bWithinUseDistance)
	{
		SetFailure(
			ECFTargetUseFailureReason::OutOfRange,
			FString::Printf(TEXT("선택 대상이 장비 최대 사용 거리 밖에 있습니다. 거리 %.1f cm / 최대 %.1f cm"), UseResult.DistanceCm, UseResult.MaxUseDistanceCm));
		return UseResult;
	}

	UseResult.bCanUseTarget = true;
	UseResult.FailureReason = ECFTargetUseFailureReason::None;
	UseResult.ResultMessage = FText::FromString(TEXT("현재 선택 대상을 장비에 사용할 수 있습니다."));
	return UseResult;
}

FString UCFTargetSelectComp::BuildTargetUseDebugSummary(const FCFTargetUseResult& UseResult) const
{
	return FString::Printf(
		TEXT("TargetUse: Equipment=%s, Target=%s, Ready=%s, HasSelection=%s, Valid=%s, Compatible=%s, InRange=%s, CanUse=%s, Distance=%.1f/%.1fcm, Track=%s, Failure=%s, Tag=%s"),
		*UseResult.EquipmentId.ToString(),
		UseResult.TargetActor ? *UseResult.TargetActor->GetName() : TEXT("None"),
		UseResult.bEquipmentReady ? TEXT("True") : TEXT("False"),
		UseResult.bHasSelectedTarget ? TEXT("True") : TEXT("False"),
		UseResult.bSelectedTargetValid ? TEXT("True") : TEXT("False"),
		UseResult.bTargetCompatible ? TEXT("True") : TEXT("False"),
		UseResult.bWithinUseDistance ? TEXT("True") : TEXT("False"),
		UseResult.bCanUseTarget ? TEXT("True") : TEXT("False"),
		UseResult.DistanceCm,
		UseResult.MaxUseDistanceCm,
		*UEnum::GetValueAsString(UseResult.TrackState),
		*UEnum::GetValueAsString(UseResult.FailureReason),
		*UseResult.FailureAttributeTag.ToString());
}

bool UCFTargetSelectComp::DoesDisplayInfoMatchContext(

	const FCFTargetDisplayInfo& DisplayInfo,
	const FCFTargetSelectionContext& SelectionContext) const
{
	if (!SelectionContext.AllowedCategories.IsEmpty()
		&& !SelectionContext.AllowedCategories.Contains(DisplayInfo.TargetCategory))
	{
		return false;
	}

	if (!SelectionContext.AllowedRelations.IsEmpty()
		&& !SelectionContext.AllowedRelations.Contains(DisplayInfo.Relation))
	{
		return false;
	}

	for (const FName RequiredTag : SelectionContext.RequiredAttributeTags)
	{
		if (!DisplayInfo.AttributeTags.Contains(RequiredTag))
		{
			return false;
		}
	}

	for (const FName ExcludedTag : SelectionContext.ExcludedAttributeTags)
	{
		if (DisplayInfo.AttributeTags.Contains(ExcludedTag))
		{
			return false;
		}
	}

	return true;
}
