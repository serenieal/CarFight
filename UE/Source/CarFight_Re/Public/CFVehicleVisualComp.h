// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-09-06
// Description: CF-FQ-048 VPS-P0-01 차량 시각 행동 전용 내부 컴포넌트
// Changelog:
// - v1.0.1: 중간검수 교정으로 BP/SCS SceneComponent 장기 포인터 bookkeeping cache를 제거하고 fresh resolve 계약을 엄격히 유지.
// - v1.0.0: Chassis/Wheel/Layout/Turret/Owner Visual 행동과 순수 시각 캐시를 Pawn에서 분리. Pawn의 기존 wrapper·상태 Authority·lifecycle ordering은 유지.
// Migration:
// - 기존 Blueprint/Public API 변경 없음. ACFVehiclePawn이 VehicleVisualComp 기본 서브오브젝트를 자동 생성하며 Product Asset 수동 추가/저장은 필요하지 않음.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFVehicleVisualComp.generated.h"

class ACFVehiclePawn;
class USceneComponent;
class UStaticMeshComponent;
struct FCFVehicleHardpointSlot;
struct FCFVehicleMountProfile;

/**
 * ACFVehiclePawn의 시각 표현 행동을 수행하는 내부 C++ 컴포넌트입니다.
 * Pawn은 기존 wrapper, observable state와 lifecycle order를 계속 소유합니다.
 */
UCLASS()
class CARFIGHT_RE_API UCFVehicleVisualComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// 차량 시각 컴포넌트의 기본 tick 비활성 설정을 초기화합니다.
	UCFVehicleVisualComp();

	// 현재 VehicleData의 차체 StaticMesh를 fresh-resolved SM_Body에 적용합니다.
	void ApplyVehicleVisualConfig();

	// 현재 VehicleData의 Wheel Visual 설정을 fresh-resolved Wheel_Mesh_*에 적용합니다.
	void ApplyVehicleWheelVisualConfig();

	// 현재 VehicleData의 Wheel Anchor 위치/회전만 fresh-resolved Wheel_Anchor_*에 적용합니다.
	void ApplyVehicleLayoutConfig();

	// 현재 Weapon/Fitting source에 맞는 단일 터렛 시각 계층을 구성하고 Pawn-owned observable을 갱신합니다.
	void ApplyVehicleTurretVisualConfig();

	// 현재 VehicleData와 Weapon runtime 기준 활성 터렛 MountProfile을 찾습니다.
	const FCFVehicleMountProfile* FindActiveTurretMountProfile() const;

	// 지정 LocationSlotId에 해당하는 현재 VehicleData 하드포인트 슬롯을 찾습니다.
	const FCFVehicleHardpointSlot* FindTurretHardpointSlot(FName LocationSlotId) const;

	// 터렛 시각 컴포넌트와 Pawn-owned 터렛 observable을 기본 Reset 상태로 되돌립니다.
	void ResetTurretVisualComponents();

	// 현재 Aim/Launcher 상태에서 터렛이 추적할 월드 방향을 계산합니다.
	FVector ResolveTurretAimWorldDirection() const;

	// 현재 Weapon turret state를 시각 Yaw/Pitch pivot에 적용합니다.
	void UpdateVehicleTurretAimVisuals(float DeltaSeconds);

	// 로컬 Owner 표시 안정화 계층을 준비합니다.
	bool PrepareOwnerVisualStabilization();

	// 지정 표시 컴포넌트를 Owner 표시 루트 아래로 안전하게 이동합니다.
	bool AttachOwnerVisualComponent(USceneComponent* VisualComponent);

	// 로컬 Owner 표시 루트의 안정화 회전을 갱신합니다.
	void UpdateOwnerVisualStabilization(float DeltaSeconds);

	// Owner 표시 안정화 상태를 기본 상태로 되돌립니다.
	void ResetOwnerVisualStabilization();

	// Owner 표시 회전의 Actor 대비 최대 지연각을 제한합니다.
	FRotator ClampOwnerVisualStabilizedRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const;

	// 로컬 SM_Body 전용 표시 안정화를 갱신합니다.
	void UpdateOwnerBodyVisualStabilization(float DeltaSeconds);

	// SM_Body 전용 표시 안정화를 기본 상대 회전으로 되돌립니다.
	void ResetOwnerBodyVisualStabilization();

	// SM_Body 표시 회전의 Actor 대비 최대 지연각을 제한합니다.
	FRotator ClampOwnerBodyVisualRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const;

	// 현재 Wheel_Mesh_* authored base relative transform을 최초 시각 mutation 전에 한 번 캡처합니다.
	void CaptureWheelVisualAuthoredBaseTransformsIfNeeded();

	// 다음 Construction에서 SCS authored Wheel_Mesh transform을 fresh capture하도록 캐시를 폐기합니다.
	void InvalidateWheelVisualAuthoredBaseTransforms();

	// Wheel_Mesh를 authored base transform으로 복원하고 optional Right fallback orientation을 적용합니다.
	void PrepareWheelVisualComponentForApply(UStaticMeshComponent* WheelMeshComponent, int32 WheelIndex, bool bUseRightFallbackCompensation);

private:
	// 이 컴포넌트를 소유한 ACFVehiclePawn을 매 호출 fresh resolve합니다.
	ACFVehiclePawn* ResolveVehiclePawn() const;

	// 이 컴포넌트를 소유한 ACFVehiclePawn을 const 형태로 매 호출 fresh resolve합니다.
	const ACFVehiclePawn* ResolveVehiclePawnConst() const;

	// Wheel Visual 최초 mutation 전에 캡처한 FL/FR/RL/RR authored base relative transform입니다.
	TArray<FTransform> WheelVisualAuthoredBaseTransforms;

	// Wheel Visual authored base transform 4개가 현재 Pawn lifetime에서 정상 캡처됐는지 여부입니다.
	bool bHasCapturedWheelVisualAuthoredBaseTransforms = false;

	// Owner 표시 안정화 계층 준비가 완료됐는지 여부입니다.
	bool bOwnerVisualStabilizationReady = false;

	// Owner 표시 안정화용 이전 프레임 표시 회전값입니다.
	FRotator SmoothedOwnerVisualRotation = FRotator::ZeroRotator;

	// Owner 표시 안정화 회전 기준값이 유효한지 여부입니다.
	bool bHasSmoothedOwnerVisualRotation = false;

	// Owner 표시 안정화 때문에 물리 루트 VehicleMesh 렌더링을 숨겼는지 여부입니다.
	bool bOwnerVisualPhysicsMeshHidden = false;

	// Owner 차체 표시 안정화가 현재 적용 가능한 상태인지 여부입니다.
	bool bOwnerBodyVisualStabilizationReady = false;

	// Owner 차체 표시 안정화용 이전 프레임 표시 회전값입니다.
	FRotator SmoothedOwnerBodyVisualRotation = FRotator::ZeroRotator;

	// Owner 차체 표시 안정화 회전 기준값이 유효한지 여부입니다.
	bool bHasSmoothedOwnerBodyVisualRotation = false;

	// Owner 차체 표시 안정화 전 SM_Body의 기본 상대 회전입니다.
	FRotator OriginalOwnerBodyVisualRelativeRotation = FRotator::ZeroRotator;

	// Owner 차체 표시 안정화 전 SM_Body 상대 회전을 저장했는지 여부입니다.
	bool bHasOriginalOwnerBodyVisualRelativeRotation = false;
};
