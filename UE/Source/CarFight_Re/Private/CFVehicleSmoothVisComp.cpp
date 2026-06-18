// Copyright Epic Games, Inc. All Rights Reserved.

// File: CFVehicleSmoothVisComp.cpp
// Version: v1.0.0
// Changelog:
// - v1.0.0: CFNetSmooth 수신 Transform을 실제 차량 Actor가 아닌 원격 Visual/Shell 표시 대상에만 적용하는 CarFight용 브리지 컴포넌트 추가.
// Migration:
// - 기존 ACFVehiclePawn에는 자동 부착하지 않는다. 차량 적용 단계에서 별도 Visual/Shell 대상과 CFNetSmooth 컴포넌트를 명시적으로 연결한다.
// Purpose: CarFight 차량 적용 코드가 NetSmoothSync 플러그인 Runtime 코드를 오염시키지 않도록 호스트 프로젝트 쪽 적용 경계를 제공한다.

#include "CFVehicleSmoothVisComp.h"

#include "CFNetSmoothComp.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

// CFNetSmooth Visual/Shell 브리지 컴포넌트의 기본 Tick과 안전 기본값을 설정한다.
UCFVehicleSmoothVisComp::UCFVehicleSmoothVisComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// 수신 Transform을 제공할 CFNetSmooth 컴포넌트를 지정한다.
void UCFVehicleSmoothVisComp::SetNetSmoothSource(UCFNetSmoothComp* InNetSmoothSourceComp)
{
	NetSmoothSourceComp = InNetSmoothSourceComp;
}

// 보간 Transform을 적용할 원격 Visual/Shell 표시 대상 SceneComponent를 지정한다.
void UCFVehicleSmoothVisComp::SetVisualTarget(USceneComponent* InVisualTargetComp)
{
	VisualTargetComp = InVisualTargetComp;
}

// 현재 설정으로 보간 Visual 적용이 가능한지 반환한다.
bool UCFVehicleSmoothVisComp::CanApplySmoothedVisual() const
{
	if (!bEnableVehicleSmoothVisual)
	{
		return false;
	}

	// 이 컴포넌트를 소유한 차량 또는 표시 Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (OwnerActor->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	if (bApplyOnlyToSimulatedProxy && OwnerActor->GetLocalRole() != ROLE_SimulatedProxy)
	{
		return false;
	}

	if (!NetSmoothSourceComp || !VisualTargetComp)
	{
		return false;
	}

	if (!NetSmoothSourceComp->HasReceivedState())
	{
		return false;
	}

	// Owner Actor의 RootComponent입니다.
	const USceneComponent* OwnerRootComp = OwnerActor->GetRootComponent();
	if (!bAllowRootVisualTarget && VisualTargetComp == OwnerRootComp)
	{
		return false;
	}

	// Visual Target이 물리 컴포넌트인지 확인하기 위한 PrimitiveComponent입니다.
	const UPrimitiveComponent* PrimitiveTargetComp = Cast<UPrimitiveComponent>(VisualTargetComp);
	if (PrimitiveTargetComp && PrimitiveTargetComp->IsSimulatingPhysics())
	{
		return false;
	}

	return true;
}

// CFNetSmooth에서 계산한 Transform을 Visual/Shell 표시 대상에 한 번 적용한다.
bool UCFVehicleSmoothVisComp::ApplySmoothedVisual(float)
{
	if (!CanApplySmoothedVisual())
	{
		return false;
	}

	// CFNetSmooth에서 계산한 월드 Transform입니다.
	FTransform SmoothedTransform = FTransform::Identity;

	// 이번 계산이 제한 외삽을 사용했는지 여부입니다.
	bool bUsedExtrapolation = false;
	if (!NetSmoothSourceComp->TryGetSmoothedTransform(SmoothedTransform, bUsedExtrapolation))
	{
		return false;
	}

	VisualTargetComp->SetWorldTransform(SmoothedTransform, false, nullptr, ETeleportType::TeleportPhysics);

	bLastApplyUsedExtrapolation = bUsedExtrapolation;
	bHasAppliedSmoothedVisual = true;
	LastAppliedVisualTransform = SmoothedTransform;

	return true;
}

// CFNetSmooth Source와 Visual Target이 모두 지정되어 있는지 반환한다.
bool UCFVehicleSmoothVisComp::HasValidVisualBinding() const
{
	return NetSmoothSourceComp && VisualTargetComp;
}

// 마지막 Visual 적용이 제한 외삽을 사용했는지 반환한다.
bool UCFVehicleSmoothVisComp::WasLastApplyUsingExtrapolation() const
{
	return bLastApplyUsedExtrapolation;
}

// 보간 Visual Transform을 한 번 이상 적용했는지 반환한다.
bool UCFVehicleSmoothVisComp::HasAppliedSmoothedVisual() const
{
	return bHasAppliedSmoothedVisual;
}

// 마지막으로 Visual/Shell 표시 대상에 적용한 Transform을 반환한다.
FTransform UCFVehicleSmoothVisComp::GetLastAppliedVisualTransform() const
{
	return LastAppliedVisualTransform;
}

// 컴포넌트 Tick에서 필요할 때 Visual/Shell 보간을 적용한다.
void UCFVehicleSmoothVisComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplySmoothedVisual(DeltaTime);
}
