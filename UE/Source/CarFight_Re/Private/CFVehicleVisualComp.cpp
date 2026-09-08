// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.2
// Date: 2026-09-07
// Description: CF-FQ-048 VPS-P0-01 차량 시각 행동 전용 내부 컴포넌트 구현
// Changelog:
// - v1.0.2: standalone non-unity compile에서 AActor::GetComponents를 직접 사용할 수 있도록 GameFramework/Actor.h 명시 include를 추가. Runtime 동작 변경 없음.
// - v1.0.1: 중간검수 교정으로 BP/SCS SceneComponent 장기 포인터 bookkeeping cache를 제거하고 각 행동의 fresh resolve 계약을 유지.
// - v1.0.0: Chassis/Wheel/Layout/Turret/Owner Visual 행동과 순수 시각 캐시를 Pawn에서 분리하고 fresh BP/SCS component resolve 계약을 유지.
// Migration:
// - v1.0.2는 IWYU/독립 컴파일 안정화만 수행하며 Vehicle Visual 런타임 계약, Blueprint/Public API와 Product Asset은 변경하지 않음.
// - Pawn의 기존 wrapper와 observable state authority를 유지하므로 Blueprint/Product Asset 수정은 필요하지 않음.

#include "CFVehicleVisualComp.h"

#include "CFEquipmentPresetData.h"
#include "CFLauncherComp.h"
#include "CFTurretMountData.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWheelSizeUtils.h"
#include "CFWheelSyncComp.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"

namespace
{
	// 이름이 일치하는 StaticMeshComponent를 Owner에서 매 호출 fresh resolve합니다.
	UStaticMeshComponent* FindStaticMeshComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// Owner에 현재 등록된 StaticMeshComponent 후보 목록입니다.
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		OwnerActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (StaticMeshComponent && StaticMeshComponent->GetFName() == ComponentName)
			{
				return StaticMeshComponent;
			}
		}

		return nullptr;
	}

	// 이름이 일치하는 SceneComponent를 Owner에서 매 호출 fresh resolve합니다.
	USceneComponent* FindSceneComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// Owner에 현재 등록된 SceneComponent 후보 목록입니다.
		TArray<USceneComponent*> SceneComponents;
		OwnerActor->GetComponents<USceneComponent>(SceneComponents);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (SceneComponent && SceneComponent->GetFName() == ComponentName)
			{
				return SceneComponent;
			}
		}

		return nullptr;
	}

	// 이름이 일치하는 SkeletalMeshComponent를 Owner에서 매 호출 fresh resolve합니다.
	USkeletalMeshComponent* FindSkeletalMeshComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// Owner에 현재 등록된 SkeletalMeshComponent 후보 목록입니다.
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		OwnerActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
		for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
		{
			if (SkeletalMeshComponent && SkeletalMeshComponent->GetFName() == ComponentName)
			{
				return SkeletalMeshComponent;
			}
		}

		return nullptr;
	}

	// Wheel Visual scale 요약 문자열에 항목을 이어 붙입니다.
	void AppendWheelMeshAutoScaleSummary(FString& InOutScaleSummary, const FString& ItemSummary)
	{
		if (!InOutScaleSummary.IsEmpty())
		{
			InOutScaleSummary += TEXT("; ");
		}

		InOutScaleSummary += ItemSummary;
	}

	// StaticMesh 로컬 바운드에서 설정된 측정 모드 기준 휠 반지름(cm)을 계산합니다.
	float MeasureWheelMeshRadiusCm(const UStaticMesh* WheelMesh, const ECFWheelMeshRadiusMeasureMode MeasureMode)
	{
		if (!WheelMesh)
		{
			return 0.0f;
		}

		// StaticMesh 에셋 로컬 공간의 원본 바운딩 박스입니다.
		const FBox WheelMeshBoundingBox = WheelMesh->GetBoundingBox();
		if (!WheelMeshBoundingBox.IsValid)
		{
			return 0.0f;
		}

		// 바운딩 박스 중심에서 각 축 끝까지의 거리입니다.
		const FVector WheelMeshBoxExtent = WheelMeshBoundingBox.GetExtent();

		switch (MeasureMode)
		{
		case ECFWheelMeshRadiusMeasureMode::AxisX:
			return WheelMeshBoxExtent.X;
		case ECFWheelMeshRadiusMeasureMode::AxisY:
			return WheelMeshBoxExtent.Y;
		case ECFWheelMeshRadiusMeasureMode::AxisZ:
			return WheelMeshBoxExtent.Z;
		case ECFWheelMeshRadiusMeasureMode::AutoMaxXZ:
		default:
			return FMath::Max(WheelMeshBoxExtent.X, WheelMeshBoxExtent.Z);
		}
	}

	// 목표 WheelRadius와 측정 반지름을 비교해 안전 범위로 제한된 표시 스케일을 계산합니다.
	bool CalculateWheelMeshScaleToRadius(
		const float TargetWheelRadiusCm,
		const float MeasuredWheelRadiusCm,
		const FCFVehicleWheelVisualConfig& WheelVisualConfig,
		float& OutWheelMeshScale)
	{
		if (TargetWheelRadiusCm <= 0.0f || MeasuredWheelRadiusCm <= KINDA_SMALL_NUMBER)
		{
			OutWheelMeshScale = 1.0f;
			return false;
		}

		// DA에서 입력한 최소 스케일을 안전 하한으로 보정한 값입니다.
		const float ConfigScaleClampMin = FMath::Max(0.01f, WheelVisualConfig.WheelMeshScaleClampMin);

		// DA에서 입력한 최대 스케일을 안전 하한으로 보정한 값입니다.
		const float ConfigScaleClampMax = FMath::Max(0.01f, WheelVisualConfig.WheelMeshScaleClampMax);

		// 최소/최대 입력이 뒤집혀도 실제 Clamp에 사용할 낮은 값입니다.
		const float SafeScaleClampMin = FMath::Min(ConfigScaleClampMin, ConfigScaleClampMax);

		// 최소/최대 입력이 뒤집혀도 실제 Clamp에 사용할 높은 값입니다.
		const float SafeScaleClampMax = FMath::Max(ConfigScaleClampMin, ConfigScaleClampMax);

		// 목표 반지름을 메시 원본 반지름으로 나눈 원본 스케일 배율입니다.
		const float RawWheelMeshScale = TargetWheelRadiusCm / MeasuredWheelRadiusCm;

		OutWheelMeshScale = FMath::Clamp(RawWheelMeshScale, SafeScaleClampMin, SafeScaleClampMax);
		return true;
	}

	// Wheel_Mesh_*에 effective mesh를 넣고 Socket-authored 또는 Legacy AutoScale 중 정확히 한 scale authority만 적용합니다.
	void ApplyWheelMeshVisualConfigToComponent(
		UStaticMeshComponent* WheelMeshComponent,
		UStaticMesh* WheelMesh,
		const FVector& WheelSocketScale,
		const float TargetWheelRadiusCm,
		const FCFVehicleWheelVisualConfig& WheelVisualConfig,
		FString& InOutScaleSummary)
	{
		if (!WheelMeshComponent)
		{
			return;
		}

		WheelMeshComponent->SetStaticMesh(WheelMesh);

		// 현재 처리 중인 Wheel_Mesh_* 컴포넌트의 표시 이름입니다.
		const FString WheelMeshComponentName = WheelMeshComponent->GetName();

		if (WheelVisualConfig.bUseWheelSocketScale)
		{
			if (!WheelMesh)
			{
				AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=SocketScaleMeshMissing"), *WheelMeshComponentName));
				return;
			}

			// Socket scale 검증 실패 원인을 받을 문자열입니다.
			FString SocketScaleError;
			if (!FCFWheelSizeUtils::ValidateWheelSocketScale(WheelSocketScale, SocketScaleError))
			{
				AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=InvalidSocketScale(%s)"), *WheelMeshComponentName, *SocketScaleError));
				return;
			}

			WheelMeshComponent->SetRelativeScale3D(WheelSocketScale);
			AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=SocketScale %s"), *WheelMeshComponentName, *WheelSocketScale.ToCompactString()));
			return;
		}

		if (!WheelVisualConfig.bAutoScaleWheelMeshToRadius)
		{
			return;
		}

		if (!WheelMesh)
		{
			AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=MeshMissing"), *WheelMeshComponentName));
			return;
		}

		// 자동 중심 보정에도 사용할 StaticMesh 원본 바운딩 박스입니다.
		const FBox WheelMeshBoundingBox = WheelMesh->GetBoundingBox();

		// StaticMesh 원본 바운드에서 계산한 휠 반지름(cm)입니다.
		const float MeasuredWheelRadiusCm = MeasureWheelMeshRadiusCm(WheelMesh, WheelVisualConfig.WheelMeshRadiusMeasureMode);

		// WheelRadius 대비 적용할 최종 Uniform Scale 값입니다.
		float FinalWheelMeshScale = 1.0f;
		if (!CalculateWheelMeshScaleToRadius(TargetWheelRadiusCm, MeasuredWheelRadiusCm, WheelVisualConfig, FinalWheelMeshScale))
		{
			AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=InvalidRadius(Target=%.2f,Measured=%.2f)"), *WheelMeshComponentName, TargetWheelRadiusCm, MeasuredWheelRadiusCm));
			return;
		}

		WheelMeshComponent->SetRelativeScale3D(FVector(FinalWheelMeshScale));

		// 자동 중심 보정 후 Wheel_Mesh 원점에 맞춰질 StaticMesh 바운드 중심입니다.
		FVector WheelMeshBoundsCenter = FVector::ZeroVector;
		if (WheelMeshBoundingBox.IsValid)
		{
			WheelMeshBoundsCenter = WheelMeshBoundingBox.GetCenter();
		}

		if (WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin)
		{
			// 컴포넌트 회전과 스케일을 반영해 바운드 중심을 부모 공간에서 원점으로 되돌리는 위치 보정값입니다.
			const FVector WheelMeshCenterCorrection = WheelMeshComponent->GetRelativeRotation().RotateVector(-WheelMeshBoundsCenter * FinalWheelMeshScale);
			WheelMeshComponent->SetRelativeLocation(WheelMeshCenterCorrection);
		}

		AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=Scale %.3f(Target=%.2f,Measured=%.2f,Center=%s,CenterFix=%s)"), *WheelMeshComponentName, FinalWheelMeshScale, TargetWheelRadiusCm, MeasuredWheelRadiusCm, *WheelMeshBoundsCenter.ToCompactString(), WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin ? TEXT("On") : TEXT("Off")));
	}
}

// 차량 시각 컴포넌트의 기본 tick 비활성 설정을 초기화합니다.
UCFVehicleVisualComp::UCFVehicleVisualComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 이 컴포넌트를 소유한 ACFVehiclePawn을 매 호출 fresh resolve합니다.
ACFVehiclePawn* UCFVehicleVisualComp::ResolveVehiclePawn() const
{
	return Cast<ACFVehiclePawn>(GetOwner());
}

// 이 컴포넌트를 소유한 ACFVehiclePawn을 const 형태로 매 호출 fresh resolve합니다.
const ACFVehiclePawn* UCFVehicleVisualComp::ResolveVehiclePawnConst() const
{
	return Cast<ACFVehiclePawn>(GetOwner());
}

// 현재 VehicleData의 차체 StaticMesh를 fresh-resolved SM_Body에 적용합니다.
void UCFVehicleVisualComp::ApplyVehicleVisualConfig()
{
	// 시각 설정을 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !VehiclePawn->VehicleData)
	{
		return;
	}

	// 현재 호출 시점에 이름으로 다시 찾은 표준 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* ChassisStaticMeshComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("SM_Body"));
	if (ChassisStaticMeshComponent && VehiclePawn->VehicleData->VehicleVisualConfig.ChassisMesh)
	{
		ChassisStaticMeshComponent->SetStaticMesh(VehiclePawn->VehicleData->VehicleVisualConfig.ChassisMesh);
	}
}

// 현재 Wheel_Mesh_* authored base relative transform을 최초 시각 mutation 전에 한 번 캡처합니다.
void UCFVehicleVisualComp::CaptureWheelVisualAuthoredBaseTransformsIfNeeded()
{
	if (bHasCapturedWheelVisualAuthoredBaseTransforms)
	{
		return;
	}

	// Wheel_Mesh_*를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	// FL/FR/RL/RR 순서와 WheelSync index 계약을 공유하는 Wheel_Mesh 컴포넌트 이름입니다.
	static const FName WheelMeshComponentNames[] =
	{
		TEXT("Wheel_Mesh_FL"),
		TEXT("Wheel_Mesh_FR"),
		TEXT("Wheel_Mesh_RL"),
		TEXT("Wheel_Mesh_RR")
	};

	// 모든 Wheel_Mesh가 존재할 때만 한 번에 commit할 임시 authored base transform 배열입니다.
	TArray<FTransform> CapturedBaseTransforms;
	CapturedBaseTransforms.Reserve(UE_ARRAY_COUNT(WheelMeshComponentNames));

	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(WheelMeshComponentNames); ++WheelIndex)
	{
		// 현재 index의 Wheel_Mesh 컴포넌트입니다.
		UStaticMeshComponent* WheelMeshComponent = FindStaticMeshComponentByName(VehiclePawn, WheelMeshComponentNames[WheelIndex]);
		if (!WheelMeshComponent)
		{
			return;
		}

		CapturedBaseTransforms.Add(WheelMeshComponent->GetRelativeTransform());
	}

	WheelVisualAuthoredBaseTransforms = MoveTemp(CapturedBaseTransforms);
	bHasCapturedWheelVisualAuthoredBaseTransforms = true;
}

// 다음 Construction에서 SCS authored Wheel_Mesh transform을 fresh capture하도록 캐시를 폐기합니다.
void UCFVehicleVisualComp::InvalidateWheelVisualAuthoredBaseTransforms()
{
	WheelVisualAuthoredBaseTransforms.Reset();
	bHasCapturedWheelVisualAuthoredBaseTransforms = false;
}

// Wheel_Mesh를 authored base transform으로 복원하고 optional Right fallback orientation을 적용합니다.
void UCFVehicleVisualComp::PrepareWheelVisualComponentForApply(UStaticMeshComponent* WheelMeshComponent, const int32 WheelIndex, const bool bUseRightFallbackCompensation)
{
	if (!WheelMeshComponent)
	{
		return;
	}

	CaptureWheelVisualAuthoredBaseTransformsIfNeeded();
	if (!bHasCapturedWheelVisualAuthoredBaseTransforms || !WheelVisualAuthoredBaseTransforms.IsValidIndex(WheelIndex))
	{
		return;
	}

	// Blueprint/Construction에서 authored된 원래 Wheel_Mesh 전체 상대 transform입니다.
	const FTransform& AuthoredBaseTransform = WheelVisualAuthoredBaseTransforms[WheelIndex];
	WheelMeshComponent->SetRelativeTransform(AuthoredBaseTransform);

	// Blueprint/Construction에서 authored된 원래 Wheel_Mesh 상대 회전입니다.
	const FQuat AuthoredBaseRotation = AuthoredBaseTransform.GetRotation();

	// Left용 FL Mesh를 Right slot에서 재사용할 때만 local X(Roll) 180도를 추가하는 회전입니다.
	const FQuat SlotOrientationCompensation = bUseRightFallbackCompensation
		? FRotator(0.0f, 0.0f, 180.0f).Quaternion()
		: FQuat::Identity;

	// 반복 Apply에서도 누적되지 않는 최종 Wheel_Mesh 상대 회전입니다.
	const FQuat TargetRelativeRotation = (AuthoredBaseRotation * SlotOrientationCompensation).GetNormalized();
	WheelMeshComponent->SetRelativeRotation(TargetRelativeRotation.Rotator());
}

// 현재 VehicleData의 Wheel Visual 설정을 fresh-resolved Wheel_Mesh_*에 적용합니다.
void UCFVehicleVisualComp::ApplyVehicleWheelVisualConfig()
{
	// Wheel Visual 입력과 결과 Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !VehiclePawn->WheelSyncComp || !VehiclePawn->VehicleData)
	{
		return;
	}

	// VehicleData 기준 휠 시각 설정입니다.
	const FCFVehicleWheelVisualConfig& WheelVisualConfig = VehiclePawn->VehicleData->WheelVisualConfig;

	// VehicleData 기준 휠 메시 참조 설정입니다.
	const FCFVehicleVisualConfig& VehicleVisualConfig = VehiclePawn->VehicleData->VehicleVisualConfig;

	// Legacy 휠 메시 자동 스케일 목표 반지름을 제공하는 이동 설정입니다.
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehiclePawn->VehicleData->VehicleMovementConfig;

	// USER Wheel Socket Scale을 제공하는 저장 레이아웃 설정입니다. Scale은 Wheel_Mesh에만 사용합니다.
	const FCFVehicleLayoutConfig& VehicleLayoutConfig = VehiclePawn->VehicleData->VehicleLayoutConfig;

	// FL 휠 메쉬의 effective source입니다.
	UStaticMesh* EffectiveWheelMeshFL = VehicleVisualConfig.WheelMeshFL;

	// FR이 비어 있을 때 FL을 재사용하는 effective source입니다.
	UStaticMesh* EffectiveWheelMeshFR = VehicleVisualConfig.WheelMeshFR ? VehicleVisualConfig.WheelMeshFR.Get() : EffectiveWheelMeshFL;

	// RL이 비어 있을 때 FL을 재사용하는 effective source입니다.
	UStaticMesh* EffectiveWheelMeshRL = VehicleVisualConfig.WheelMeshRL ? VehicleVisualConfig.WheelMeshRL.Get() : EffectiveWheelMeshFL;

	// RR이 비어 있을 때 FL을 재사용하는 effective source입니다.
	UStaticMesh* EffectiveWheelMeshRR = VehicleVisualConfig.WheelMeshRR ? VehicleVisualConfig.WheelMeshRR.Get() : EffectiveWheelMeshFL;

	// FR이 explicit Mesh 없이 FL을 실제 fallback source로 사용하는지 여부입니다.
	const bool bFrontRightUsesLeftFallback = VehicleVisualConfig.WheelMeshFR == nullptr && EffectiveWheelMeshFL != nullptr;

	// RR이 explicit Mesh 없이 FL을 실제 fallback source로 사용하는지 여부입니다.
	const bool bRearRightUsesLeftFallback = VehicleVisualConfig.WheelMeshRR == nullptr && EffectiveWheelMeshFL != nullptr;

	VehiclePawn->WheelSyncComp->ExpectedWheelCount = WheelVisualConfig.ExpectedWheelCount;
	VehiclePawn->WheelSyncComp->FrontWheelCountForSteering = WheelVisualConfig.FrontWheelCountForSteering;

	// FL/RL은 +1, Right FL fallback만 -1로 설정하는 per-wheel runtime spin handedness입니다.
	TArray<float> WheelSpinHandednessSigns;
	WheelSpinHandednessSigns.Init(1.0f, 4);
	WheelSpinHandednessSigns[1] = bFrontRightUsesLeftFallback ? -1.0f : 1.0f;
	WheelSpinHandednessSigns[3] = bRearRightUsesLeftFallback ? -1.0f : 1.0f;
	VehiclePawn->WheelSyncComp->SetWheelSpinHandednessSigns(WheelSpinHandednessSigns);

	CaptureWheelVisualAuthoredBaseTransformsIfNeeded();

	// 런타임 요약에 남길 현재 Wheel Size 시각 authority입니다. Socket mode가 Legacy AutoScale보다 우선합니다.
	FString WheelMeshAutoScaleSummary = WheelVisualConfig.bUseWheelSocketScale
		? TEXT("SocketScale=On")
		: (WheelVisualConfig.bAutoScaleWheelMeshToRadius ? TEXT("AutoScale=On") : TEXT("ScaleAuthority=Manual"));

	// 앞왼쪽 Wheel_Mesh 컴포넌트입니다.
	UStaticMeshComponent* WheelMeshFLComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("Wheel_Mesh_FL"));
	if (WheelMeshFLComponent)
	{
		PrepareWheelVisualComponentForApply(WheelMeshFLComponent, 0, false);
		ApplyWheelMeshVisualConfigToComponent(WheelMeshFLComponent, EffectiveWheelMeshFL, VehicleLayoutConfig.WheelAnchorFL.RelativeScale, VehicleMovementConfig.FrontWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	// 앞오른쪽 Wheel_Mesh 컴포넌트입니다.
	UStaticMeshComponent* WheelMeshFRComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("Wheel_Mesh_FR"));
	if (WheelMeshFRComponent)
	{
		PrepareWheelVisualComponentForApply(WheelMeshFRComponent, 1, bFrontRightUsesLeftFallback);
		ApplyWheelMeshVisualConfigToComponent(WheelMeshFRComponent, EffectiveWheelMeshFR, VehicleLayoutConfig.WheelAnchorFR.RelativeScale, VehicleMovementConfig.FrontWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	// 뒤왼쪽 Wheel_Mesh 컴포넌트입니다.
	UStaticMeshComponent* WheelMeshRLComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("Wheel_Mesh_RL"));
	if (WheelMeshRLComponent)
	{
		PrepareWheelVisualComponentForApply(WheelMeshRLComponent, 2, false);
		ApplyWheelMeshVisualConfigToComponent(WheelMeshRLComponent, EffectiveWheelMeshRL, VehicleLayoutConfig.WheelAnchorRL.RelativeScale, VehicleMovementConfig.RearWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	// 뒤오른쪽 Wheel_Mesh 컴포넌트입니다.
	UStaticMeshComponent* WheelMeshRRComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("Wheel_Mesh_RR"));
	if (WheelMeshRRComponent)
	{
		PrepareWheelVisualComponentForApply(WheelMeshRRComponent, 3, bRearRightUsesLeftFallback);
		ApplyWheelMeshVisualConfigToComponent(WheelMeshRRComponent, EffectiveWheelMeshRR, VehicleLayoutConfig.WheelAnchorRR.RelativeScale, VehicleMovementConfig.RearWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	VehiclePawn->LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: WheelVisual ExpectedWheelCount=%d, FrontWheelCount=%d, %s"), VehiclePawn->WheelSyncComp->ExpectedWheelCount, VehiclePawn->WheelSyncComp->FrontWheelCountForSteering, *WheelMeshAutoScaleSummary);
}

// 현재 VehicleData의 Wheel Anchor 위치/회전만 fresh-resolved Wheel_Anchor_*에 적용합니다.
void UCFVehicleVisualComp::ApplyVehicleLayoutConfig()
{
	// Layout 입력과 summary Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	if (!VehiclePawn->VehicleData)
	{
		VehiclePawn->LastVehicleRuntimeSummary = TEXT("VehicleLayout: VehicleData=Missing, ManualAnchorLayout=Fallback");
		return;
	}

	// VehicleData에서 읽은 차량 레이아웃 설정입니다.
	const FCFVehicleLayoutConfig& VehicleLayoutConfig = VehiclePawn->VehicleData->VehicleLayoutConfig;
	if (!VehicleLayoutConfig.bUseLayoutOverrides)
	{
		VehiclePawn->LastVehicleRuntimeSummary = TEXT("VehicleLayout: ManualAnchorLayout=Fallback");
		return;
	}

	// 레이아웃을 적용하지 못한 Wheel_Anchor_* 컴포넌트 이름 목록입니다.
	FString MissingWheelAnchorNames;

	// DataAsset 레이아웃이 적용된 바퀴 앵커 개수입니다.
	int32 AppliedWheelAnchorCount = 0;

	// 단일 WheelAnchor 위치/회전을 같은 이름의 SceneComponent에 적용하며 Scale은 변경하지 않는 로컬 함수입니다.
	const auto ApplyWheelAnchorPose = [VehiclePawn, &MissingWheelAnchorNames, &AppliedWheelAnchorCount](const FName WheelAnchorName, const FCFWheelAnchorPose& WheelAnchorPose)
	{
		// 이름으로 fresh resolve한 바퀴 앵커 SceneComponent입니다.
		USceneComponent* WheelAnchorComponent = FindSceneComponentByName(VehiclePawn, WheelAnchorName);
		if (!WheelAnchorComponent)
		{
			if (!MissingWheelAnchorNames.IsEmpty())
			{
				MissingWheelAnchorNames += TEXT(", ");
			}
			MissingWheelAnchorNames += WheelAnchorName.ToString();
			return;
		}

#if WITH_EDITOR
		if (GIsEditor)
		{
			WheelAnchorComponent->Modify();
		}
#endif

		WheelAnchorComponent->SetRelativeLocationAndRotation(WheelAnchorPose.RelativeLocation, WheelAnchorPose.RelativeRotation, false, nullptr, ETeleportType::TeleportPhysics);
		WheelAnchorComponent->UpdateComponentToWorld();
		++AppliedWheelAnchorCount;
	};

	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_FL"), VehicleLayoutConfig.WheelAnchorFL);
	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_FR"), VehicleLayoutConfig.WheelAnchorFR);
	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_RL"), VehicleLayoutConfig.WheelAnchorRL);
	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_RR"), VehicleLayoutConfig.WheelAnchorRR);

	if (!MissingWheelAnchorNames.IsEmpty())
	{
		VehiclePawn->LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleLayout: LayoutOverride=Partial, Applied=%d, Missing=%s"), AppliedWheelAnchorCount, *MissingWheelAnchorNames);
		return;
	}

	VehiclePawn->LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleLayout: LayoutOverride=Applied, Applied=%d"), AppliedWheelAnchorCount);
}

// 현재 VehicleData와 Weapon runtime 기준 활성 터렛 MountProfile을 찾습니다.
const FCFVehicleMountProfile* UCFVehicleVisualComp::FindActiveTurretMountProfile() const
{
	// 터렛 MountProfile source를 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	if (!VehiclePawn || !VehiclePawn->VehicleData)
	{
		return nullptr;
	}

	// VehicleWeaponComp가 우선 사용하는 활성 장착 프로파일 ID입니다.
	const FName RequestedMountProfileId = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveMountProfileId() : FName(TEXT("RoofTurret_MediumOrLarge"));

	for (const FCFVehicleMountProfile& MountProfile : VehiclePawn->VehicleData->MountProfiles)
	{
		if (MountProfile.MountProfileId == RequestedMountProfileId)
		{
			return &MountProfile;
		}
	}

	if (RequestedMountProfileId.IsNone() && !VehiclePawn->VehicleData->MountProfiles.IsEmpty())
	{
		return &VehiclePawn->VehicleData->MountProfiles[0];
	}

	return nullptr;
}

// 지정 LocationSlotId에 해당하는 현재 VehicleData 하드포인트 슬롯을 찾습니다.
const FCFVehicleHardpointSlot* UCFVehicleVisualComp::FindTurretHardpointSlot(const FName LocationSlotId) const
{
	// 하드포인트 source를 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	if (!VehiclePawn || !VehiclePawn->VehicleData || LocationSlotId.IsNone())
	{
		return nullptr;
	}

	for (const FCFVehicleHardpointSlot& HardpointSlot : VehiclePawn->VehicleData->HardpointSlots)
	{
		if (HardpointSlot.LocationSlotId == LocationSlotId)
		{
			return &HardpointSlot;
		}
	}

	return nullptr;
}

// 터렛 시각 컴포넌트와 Pawn-owned 터렛 observable을 기본 Reset 상태로 되돌립니다.
void UCFVehicleVisualComp::ResetTurretVisualComponents()
{
	// 터렛 시각 계층과 observable Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	VehiclePawn->LastTurretMountData = nullptr;
	VehiclePawn->bLastTurretMountDataAssigned = false;
	VehiclePawn->LastTurretMountId = NAME_None;
	VehiclePawn->LastTurretMountSummary = TEXT("TurretMountData: Reset");
	VehiclePawn->bLastTurretVisualAttached = false;
	VehiclePawn->LastTurretVisualSummary = TEXT("TurretVisual: Reset");
	VehiclePawn->LastTurretBaseMeshName = NAME_None;
	VehiclePawn->LastTurretYawMeshName = NAME_None;
	VehiclePawn->LastTurretPitchMeshName = NAME_None;

	if (VehiclePawn->TurretMountRootComp)
	{
		VehiclePawn->TurretMountRootComp->SetRelativeTransform(FTransform::Identity);
		VehiclePawn->TurretMountRootComp->SetVisibility(false, true);
		VehiclePawn->TurretMountRootComp->SetHiddenInGame(true, true);
	}

	if (VehiclePawn->TurretBaseMeshComp)
	{
		VehiclePawn->TurretBaseMeshComp->AttachToComponent(VehiclePawn->TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretBaseMeshComp->SetStaticMesh(nullptr);
		VehiclePawn->TurretBaseMeshComp->SetRelativeTransform(FTransform::Identity);
		VehiclePawn->TurretBaseMeshComp->SetVisibility(false, true);
		VehiclePawn->TurretBaseMeshComp->SetHiddenInGame(true, true);
	}

	if (VehiclePawn->TurretYawPivotComp)
	{
		VehiclePawn->TurretYawPivotComp->AttachToComponent(VehiclePawn->TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretYawPivotComp->SetRelativeTransform(FTransform::Identity);
	}

	if (VehiclePawn->TurretYawMeshComp)
	{
		VehiclePawn->TurretYawMeshComp->AttachToComponent(VehiclePawn->TurretYawPivotComp ? VehiclePawn->TurretYawPivotComp.Get() : VehiclePawn->TurretMountRootComp.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretYawMeshComp->SetStaticMesh(nullptr);
		VehiclePawn->TurretYawMeshComp->SetRelativeTransform(FTransform::Identity);
		VehiclePawn->TurretYawMeshComp->SetVisibility(false, true);
		VehiclePawn->TurretYawMeshComp->SetHiddenInGame(true, true);
	}

	if (VehiclePawn->TurretPitchPivotComp)
	{
		VehiclePawn->TurretPitchPivotComp->AttachToComponent(VehiclePawn->TurretYawMeshComp ? static_cast<USceneComponent*>(VehiclePawn->TurretYawMeshComp.Get()) : VehiclePawn->TurretYawPivotComp.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretPitchPivotComp->SetRelativeTransform(FTransform::Identity);
	}

	if (VehiclePawn->TurretPitchMeshComp)
	{
		VehiclePawn->TurretPitchMeshComp->AttachToComponent(VehiclePawn->TurretPitchPivotComp ? VehiclePawn->TurretPitchPivotComp.Get() : VehiclePawn->TurretMountRootComp.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretPitchMeshComp->SetStaticMesh(nullptr);
		VehiclePawn->TurretPitchMeshComp->SetRelativeTransform(FTransform::Identity);
		VehiclePawn->TurretPitchMeshComp->SetVisibility(false, true);
		VehiclePawn->TurretPitchMeshComp->SetHiddenInGame(true, true);
	}
}

// 현재 Weapon/Fitting source에 맞는 단일 터렛 시각 계층을 구성하고 Pawn-owned observable을 갱신합니다.
void UCFVehicleVisualComp::ApplyVehicleTurretVisualConfig()
{
	// 터렛 시각 계층과 observable Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	ResetTurretVisualComponents();

	if (!VehiclePawn->VehicleData)
	{
		VehiclePawn->LastTurretVisualSummary = TEXT("TurretVisual: VehicleData=Missing");
		return;
	}

	if (!VehiclePawn->TurretMountRootComp || !VehiclePawn->TurretBaseMeshComp || !VehiclePawn->TurretYawPivotComp || !VehiclePawn->TurretYawMeshComp || !VehiclePawn->TurretPitchPivotComp || !VehiclePawn->TurretPitchMeshComp)
	{
		VehiclePawn->LastTurretVisualSummary = TEXT("TurretVisual: Components=Missing");
		return;
	}

	// 현재 터렛 시각 표시를 적용할 활성 장착 프로파일입니다.
	const FCFVehicleMountProfile* ActiveMountProfile = FindActiveTurretMountProfile();
	if (!ActiveMountProfile)
	{
		VehiclePawn->LastTurretVisualSummary = TEXT("TurretVisual: MountProfile=Missing");
		return;
	}

	if (ActiveMountProfile->MountType != ECFVehicleMountType::Turret)
	{
		VehiclePawn->LastTurretVisualSummary = FString::Printf(TEXT("TurretVisual: SkippedNonTurret, Profile=%s"), *ActiveMountProfile->MountProfileId.ToString());
		return;
	}

	// 활성 장착 프로파일이 참조하는 하드포인트 슬롯입니다.
	const FCFVehicleHardpointSlot* HardpointSlot = FindTurretHardpointSlot(ActiveMountProfile->LocationSlotRef);
	if (!HardpointSlot)
	{
		VehiclePawn->LastTurretVisualSummary = FString::Printf(TEXT("TurretVisual: HardpointSlot=Missing, LocationSlotRef=%s"), *ActiveMountProfile->LocationSlotRef.ToString());
		return;
	}

	// Weapon Runtime 준비 후에는 Commit된 Snapshot EquipmentPresetData를 우선하는 최종 preset입니다.
	UCFEquipmentPresetData* ResolvedEquipmentPresetData = VehiclePawn->VehicleWeaponComp
		&& VehiclePawn->VehicleWeaponComp->IsWeaponRuntimeReady()
		&& VehiclePawn->VehicleWeaponComp->GetActiveMountProfileId() == ActiveMountProfile->MountProfileId
		? VehiclePawn->VehicleWeaponComp->GetActiveEquipmentPresetData()
		: ActiveMountProfile->DefaultEquipmentPresetData.Get();

	// 최종 EquipmentPresetData에서 TurretMountData를 해석했는지 여부입니다.
	const bool bUsingEquipmentPresetTurretMountData = ResolvedEquipmentPresetData
		&& ResolvedEquipmentPresetData->DefaultTurretMountData;

	// Legacy 기본값 또는 Snapshot Override의 활성 TurretMountData입니다.
	UCFTurretMountData* ActiveTurretMountData = bUsingEquipmentPresetTurretMountData
		? ResolvedEquipmentPresetData->DefaultTurretMountData.Get()
		: nullptr;

	VehiclePawn->LastTurretMountData = ActiveTurretMountData;
	VehiclePawn->bLastTurretMountDataAssigned = ActiveTurretMountData != nullptr;
	VehiclePawn->LastTurretMountId = ActiveTurretMountData ? ActiveTurretMountData->TurretMountId : NAME_None;
	VehiclePawn->LastTurretMountSummary = ActiveTurretMountData ? ActiveTurretMountData->BuildTurretMountSummary() : TEXT("TurretMountData: Missing, Source=EquipmentPresetDataRequired, InlineFallback=Removed");

	// 터렛 시각 값을 가져온 원본을 표시할 문자열입니다.
	const FString TurretVisualSourceText = bUsingEquipmentPresetTurretMountData
		? (VehiclePawn->VehicleWeaponComp && VehiclePawn->VehicleWeaponComp->IsUsingRuntimeEquipmentPresetOverride()
			? TEXT("FittingSnapshotEquipmentPresetData")
			: TEXT("VehicleDefaultEquipmentPresetData"))
		: TEXT("MissingEquipmentPresetTurretMountData");

	// 현재 적용할 Base 메쉬입니다.
	UStaticMesh* ResolvedTurretBaseMesh = ActiveTurretMountData ? ActiveTurretMountData->TurretBaseMesh.Get() : nullptr;

	// 현재 적용할 Yaw 메쉬입니다.
	UStaticMesh* ResolvedTurretYawMesh = ActiveTurretMountData ? ActiveTurretMountData->TurretYawMesh.Get() : nullptr;

	// 현재 적용할 Pitch 메쉬입니다.
	UStaticMesh* ResolvedTurretPitchMesh = ActiveTurretMountData ? ActiveTurretMountData->TurretPitchMesh.Get() : nullptr;

	// 현재 적용할 Base 메쉬 상대 Transform입니다.
	const FTransform ResolvedTurretBaseRelativeTransform = ActiveTurretMountData ? ActiveTurretMountData->TurretBaseRelativeTransform : FTransform::Identity;

	// 현재 적용할 Yaw 피벗 소켓 이름입니다.
	const FName ResolvedYawPivotSocketName = ActiveTurretMountData ? ActiveTurretMountData->YawPivotSocketName : NAME_None;

	// 현재 적용할 Yaw 메쉬 상대 Transform입니다.
	const FTransform ResolvedTurretYawRelativeTransform = ActiveTurretMountData ? ActiveTurretMountData->TurretYawRelativeTransform : FTransform::Identity;

	// 현재 적용할 Pitch 메쉬 상대 Transform입니다.
	const FTransform ResolvedTurretPitchRelativeTransform = ActiveTurretMountData ? ActiveTurretMountData->TurretPitchRelativeTransform : FTransform::Identity;

	// 현재 적용할 Pitch 피벗 소켓 이름입니다.
	const FName ResolvedPitchPivotSocketName = ActiveTurretMountData ? ActiveTurretMountData->PitchPivotSocketName : NAME_None;

	// Muzzle FireOrigin 전환에서 사용할 소켓 이름입니다.
	const FName ResolvedMuzzleSocketName = ActiveTurretMountData ? ActiveTurretMountData->MuzzleSocketName : NAME_None;

	// 활성 터렛 마운트 소스에 Base 메쉬가 지정되어 있는지 여부입니다.
	const bool bHasBaseMesh = ResolvedTurretBaseMesh != nullptr;

	// 활성 터렛 마운트 소스에 Yaw 메쉬가 지정되어 있는지 여부입니다.
	const bool bHasYawMesh = ResolvedTurretYawMesh != nullptr;

	// 활성 터렛 마운트 소스에 Pitch 메쉬가 지정되어 있는지 여부입니다.
	const bool bHasPitchMesh = ResolvedTurretPitchMesh != nullptr;

	// 이름이 지정됐지만 실제 메쉬에서 찾지 못한 필수 소켓 목록입니다.
	TArray<FString> MissingRequiredSocketDescriptions;

	if (!bHasBaseMesh && !bHasYawMesh && !bHasPitchMesh)
	{
		VehiclePawn->LastTurretVisualSummary = FString::Printf(
			TEXT("TurretVisual: MeshMissingOptional, Profile=%s, Slot=%s, MountData=%s, Source=%s"),
			*ActiveMountProfile->MountProfileId.ToString(),
			*HardpointSlot->LocationSlotId.ToString(),
			*VehiclePawn->LastTurretMountId.ToString(),
			*TurretVisualSourceText);
		return;
	}

	// 터렛 장착 위치의 부모가 될 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("SM_Body"));

	// 차체 표시 컴포넌트가 없을 때 사용할 fallback 부모 컴포넌트입니다.
	USceneComponent* MountParentComponent = BodyMeshComponent ? Cast<USceneComponent>(BodyMeshComponent) : VehiclePawn->GetRootComponent();
	if (!MountParentComponent)
	{
		VehiclePawn->LastTurretVisualSummary = TEXT("TurretVisual: MountParent=Missing");
		return;
	}

	// 하드포인트 슬롯에 저장된 차량/차체 기준 상대 Transform입니다.
	const FTransform HardpointLocalTransform(HardpointSlot->LocalRotation, HardpointSlot->LocalLocation);

	// 터렛 루트가 맞아야 하는 하드포인트 월드 위치입니다.
	FVector ExpectedHardpointWorldLocation = (HardpointLocalTransform * MountParentComponent->GetComponentTransform()).GetLocation();

	// 하드포인트 소켓을 실제 차체 소켓으로 해결했는지 여부입니다.
	bool bHardpointSocketResolved = false;

	// 하드포인트 슬롯에 소켓 이름이 명시되어 있는지 여부입니다.
	const bool bHardpointSocketNameConfigured = !HardpointSlot->SocketName.IsNone();

	if (BodyMeshComponent && !HardpointSlot->SocketName.IsNone() && BodyMeshComponent->DoesSocketExist(HardpointSlot->SocketName))
	{
		// 실제 차체 소켓의 월드 Transform입니다.
		const FTransform HardpointSocketWorldTransform = BodyMeshComponent->GetSocketTransform(HardpointSlot->SocketName, RTS_World);

		ExpectedHardpointWorldLocation = HardpointSocketWorldTransform.GetLocation();
		VehiclePawn->TurretMountRootComp->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HardpointSlot->SocketName);
		VehiclePawn->TurretMountRootComp->SetRelativeTransform(FTransform::Identity);
		bHardpointSocketResolved = true;
	}
	else
	{
		VehiclePawn->TurretMountRootComp->AttachToComponent(MountParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretMountRootComp->SetRelativeTransform(HardpointLocalTransform);
	}

	if (bHardpointSocketNameConfigured && !bHardpointSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("Hardpoint:%s on SM_Body"), *HardpointSlot->SocketName.ToString()));
	}

	VehiclePawn->TurretMountRootComp->UpdateComponentToWorld();

	// 실제 터렛 루트 월드 위치입니다.
	const FVector TurretRootWorldLocation = VehiclePawn->TurretMountRootComp->GetComponentLocation();

	// 기대 하드포인트 위치와 실제 터렛 루트 위치 사이의 거리입니다.
	const float TurretRootToHardpointDistance = FVector::Dist(TurretRootWorldLocation, ExpectedHardpointWorldLocation);

	VehiclePawn->TurretMountRootComp->SetVisibility(true, true);
	VehiclePawn->TurretMountRootComp->SetHiddenInGame(false, true);

	// YawPivot이 Base 메쉬의 YawPivot 소켓에 붙었는지 여부입니다.
	bool bYawPivotSocketResolved = false;

	// Base 메쉬 아래에 Yaw 또는 Pitch 시각 메쉬를 붙여야 해서 YawPivot 소켓이 필요한지 여부입니다.
	const bool bYawPivotSocketRequired = bHasBaseMesh && (bHasYawMesh || bHasPitchMesh) && !ResolvedYawPivotSocketName.IsNone();

	if (bHasBaseMesh)
	{
		VehiclePawn->TurretBaseMeshComp->AttachToComponent(VehiclePawn->TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretBaseMeshComp->SetStaticMesh(ResolvedTurretBaseMesh);
		VehiclePawn->TurretBaseMeshComp->SetRelativeTransform(ResolvedTurretBaseRelativeTransform);
		VehiclePawn->TurretBaseMeshComp->SetVisibility(true, true);
		VehiclePawn->TurretBaseMeshComp->SetHiddenInGame(false, true);
		VehiclePawn->LastTurretBaseMeshName = ResolvedTurretBaseMesh->GetFName();

		if (!ResolvedYawPivotSocketName.IsNone() && VehiclePawn->TurretBaseMeshComp->DoesSocketExist(ResolvedYawPivotSocketName))
		{
			VehiclePawn->TurretYawPivotComp->AttachToComponent(VehiclePawn->TurretBaseMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ResolvedYawPivotSocketName);
			bYawPivotSocketResolved = true;
		}
		else
		{
			VehiclePawn->TurretYawPivotComp->AttachToComponent(VehiclePawn->TurretBaseMeshComp, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	else
	{
		VehiclePawn->TurretYawPivotComp->AttachToComponent(VehiclePawn->TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
	}

	VehiclePawn->TurretYawPivotComp->SetRelativeTransform(FTransform::Identity);

	if (bYawPivotSocketRequired && !bYawPivotSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("YawPivot:%s on %s"), *ResolvedYawPivotSocketName.ToString(), *VehiclePawn->LastTurretBaseMeshName.ToString()));
	}

	if (bHasYawMesh)
	{
		VehiclePawn->TurretYawMeshComp->AttachToComponent(VehiclePawn->TurretYawPivotComp, FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretYawMeshComp->SetStaticMesh(ResolvedTurretYawMesh);
		VehiclePawn->TurretYawMeshComp->SetRelativeTransform(ResolvedTurretYawRelativeTransform);
		VehiclePawn->TurretYawMeshComp->SetVisibility(true, true);
		VehiclePawn->TurretYawMeshComp->SetHiddenInGame(false, true);
		VehiclePawn->LastTurretYawMeshName = ResolvedTurretYawMesh->GetFName();
	}

	// Pitch 메쉬가 Yaw 메쉬의 피벗 소켓에 붙었는지 여부입니다.
	bool bPitchSocketResolved = false;

	// Yaw 메쉬 아래에 Pitch 메쉬를 붙여야 해서 PitchPivot 소켓이 필요한지 여부입니다.
	const bool bPitchPivotSocketRequired = bHasYawMesh && bHasPitchMesh && !ResolvedPitchPivotSocketName.IsNone();

	// PitchPivot을 붙일 기본 부모 컴포넌트입니다.
	USceneComponent* PitchPivotParentComponent = bHasYawMesh ? static_cast<USceneComponent*>(VehiclePawn->TurretYawMeshComp.Get()) : VehiclePawn->TurretYawPivotComp.Get();

	if (bHasYawMesh && !ResolvedPitchPivotSocketName.IsNone() && VehiclePawn->TurretYawMeshComp->DoesSocketExist(ResolvedPitchPivotSocketName))
	{
		VehiclePawn->TurretPitchPivotComp->AttachToComponent(VehiclePawn->TurretYawMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ResolvedPitchPivotSocketName);
		bPitchSocketResolved = true;
	}
	else
	{
		VehiclePawn->TurretPitchPivotComp->AttachToComponent(PitchPivotParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	VehiclePawn->TurretPitchPivotComp->SetRelativeTransform(FTransform::Identity);

	if (bPitchPivotSocketRequired && !bPitchSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("PitchPivot:%s on %s"), *ResolvedPitchPivotSocketName.ToString(), *VehiclePawn->LastTurretYawMeshName.ToString()));
	}

	if (bHasPitchMesh)
	{
		VehiclePawn->TurretPitchMeshComp->AttachToComponent(VehiclePawn->TurretPitchPivotComp, FAttachmentTransformRules::KeepRelativeTransform);
		VehiclePawn->TurretPitchMeshComp->SetStaticMesh(ResolvedTurretPitchMesh);
		VehiclePawn->TurretPitchMeshComp->SetRelativeTransform(ResolvedTurretPitchRelativeTransform);
		VehiclePawn->TurretPitchMeshComp->SetVisibility(true, true);
		VehiclePawn->TurretPitchMeshComp->SetHiddenInGame(false, true);
		VehiclePawn->LastTurretPitchMeshName = ResolvedTurretPitchMesh->GetFName();
	}

	// Pitch 메쉬에 Muzzle 소켓이 실제로 존재하는지 여부입니다.
	bool bMuzzleSocketResolved = false;

	// Muzzle FireOrigin 전환을 위해 Muzzle 소켓을 필수로 볼 수 있는지 여부입니다.
	const bool bMuzzleSocketRequired = bHasPitchMesh && !ResolvedMuzzleSocketName.IsNone();

	if (bMuzzleSocketRequired && VehiclePawn->TurretPitchMeshComp->DoesSocketExist(ResolvedMuzzleSocketName))
	{
		bMuzzleSocketResolved = true;
	}

	if (bMuzzleSocketRequired && !bMuzzleSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("Muzzle:%s on %s"), *ResolvedMuzzleSocketName.ToString(), *VehiclePawn->LastTurretPitchMeshName.ToString()));
	}

	// 하드포인트 소켓의 최종 부착 상태를 디버그에 표시할 문자열입니다.
	FString HardpointSocketStatusText = TEXT("MissingRequiredSocketFallback");
	if (HardpointSlot->SocketName.IsNone())
	{
		HardpointSocketStatusText = TEXT("SkippedNoSocketName");
	}
	else if (bHardpointSocketResolved)
	{
		HardpointSocketStatusText = TEXT("Resolved");
	}

	// YawPivot 소켓의 최종 부착 상태를 디버그에 표시할 문자열입니다.
	FString YawPivotStatusText = TEXT("MissingRequiredSocketFallback");
	if (!bHasBaseMesh)
	{
		YawPivotStatusText = TEXT("FallbackNoBaseMesh");
	}
	else if (!(bHasYawMesh || bHasPitchMesh))
	{
		YawPivotStatusText = TEXT("SkippedNoChildMesh");
	}
	else if (ResolvedYawPivotSocketName.IsNone())
	{
		YawPivotStatusText = TEXT("FallbackNoSocketName");
	}
	else if (bYawPivotSocketResolved)
	{
		YawPivotStatusText = TEXT("Resolved");
	}

	// PitchPivot 소켓의 최종 부착 상태를 디버그에 표시할 문자열입니다.
	FString PitchPivotStatusText = TEXT("MissingRequiredSocketFallback");
	if (!bHasPitchMesh)
	{
		PitchPivotStatusText = TEXT("SkippedNoPitchMesh");
	}
	else if (!bHasYawMesh)
	{
		PitchPivotStatusText = TEXT("FallbackNoYawMesh");
	}
	else if (ResolvedPitchPivotSocketName.IsNone())
	{
		PitchPivotStatusText = TEXT("FallbackNoSocketName");
	}
	else if (bPitchSocketResolved)
	{
		PitchPivotStatusText = TEXT("Resolved");
	}

	// Muzzle 소켓의 최종 FireOrigin 전환 상태를 디버그에 표시할 문자열입니다.
	FString MuzzleSocketStatusText = TEXT("MissingRequiredSocketFallback");
	if (!bHasPitchMesh)
	{
		MuzzleSocketStatusText = TEXT("FallbackNoPitchMesh");
	}
	else if (ResolvedMuzzleSocketName.IsNone())
	{
		MuzzleSocketStatusText = TEXT("FallbackNoSocketName");
	}
	else if (bMuzzleSocketResolved)
	{
		MuzzleSocketStatusText = TEXT("Resolved");
	}

	// 필수 소켓 검증의 전체 상태를 디버그에 표시할 문자열입니다.
	const FString SocketValidationStatusText = MissingRequiredSocketDescriptions.IsEmpty() ? TEXT("OK") : TEXT("MissingRequiredSocket");

	// 누락된 필수 소켓 목록을 한 줄 요약으로 묶은 문자열입니다.
	const FString MissingRequiredSocketSummary = MissingRequiredSocketDescriptions.IsEmpty() ? TEXT("None") : FString::Join(MissingRequiredSocketDescriptions, TEXT(" | "));

	if (!MissingRequiredSocketDescriptions.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TurretVisual missing required socket(s): %s"), *MissingRequiredSocketSummary);
	}

	VehiclePawn->bLastTurretVisualAttached = true;
	VehiclePawn->LastTurretVisualSummary = FString::Printf(
		TEXT("TurretVisual: Attached, Profile=%s, Slot=%s, HardpointSocket=%s, HardpointSocketResolved=%s, HardpointSocketStatus=%s, SocketValidation=%s, MissingRequiredSockets=%s, RootWorld=(%.1f, %.1f, %.1f), ExpectedWorld=(%.1f, %.1f, %.1f), RootDelta=%.2f, Parent=%s, MountData=%s, Source=%s, BaseMesh=%s, YawMesh=%s, PitchMesh=%s, YawPivot=%s, PitchPivot=%s, MuzzleSocket=%s, MuzzleStatus=%s"),
		*ActiveMountProfile->MountProfileId.ToString(),
		*HardpointSlot->LocationSlotId.ToString(),
		*HardpointSlot->SocketName.ToString(),
		bHardpointSocketResolved ? TEXT("Yes") : TEXT("No"),
		*HardpointSocketStatusText,
		*SocketValidationStatusText,
		*MissingRequiredSocketSummary,
		TurretRootWorldLocation.X,
		TurretRootWorldLocation.Y,
		TurretRootWorldLocation.Z,
		ExpectedHardpointWorldLocation.X,
		ExpectedHardpointWorldLocation.Y,
		ExpectedHardpointWorldLocation.Z,
		TurretRootToHardpointDistance,
		*MountParentComponent->GetName(),
		*VehiclePawn->LastTurretMountId.ToString(),
		*TurretVisualSourceText,
		*VehiclePawn->LastTurretBaseMeshName.ToString(),
		*VehiclePawn->LastTurretYawMeshName.ToString(),
		*VehiclePawn->LastTurretPitchMeshName.ToString(),
		*YawPivotStatusText,
		*PitchPivotStatusText,
		*ResolvedMuzzleSocketName.ToString(),
		*MuzzleSocketStatusText);
}

// 현재 Aim/Launcher 상태에서 터렛이 추적할 월드 방향을 계산합니다.
FVector UCFVehicleVisualComp::ResolveTurretAimWorldDirection() const
{
	// Aim source와 fallback Actor 방향을 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	if (!VehiclePawn)
	{
		return FVector::ForwardVector;
	}

	// Ripple·Salvo 진행 중 첫 입력 순간 고정된 Command Target 위치입니다.
	FVector ActiveLauncherCommandTargetLocation;
	if (VehiclePawn->LauncherComp
		&& VehiclePawn->LauncherComp->TryGetActiveCommandTargetLocation(ActiveLauncherCommandTargetLocation)
		&& VehiclePawn->TurretYawPivotComp)
	{
		// 현재 Yaw pivot에서 고정 Launcher 목표까지의 방향입니다.
		const FVector DirectionToLauncherCommandTarget = ActiveLauncherCommandTargetLocation - VehiclePawn->TurretYawPivotComp->GetComponentLocation();
		if (!DirectionToLauncherCommandTarget.ContainsNaN() && !DirectionToLauncherCommandTarget.IsNearlyZero())
		{
			return DirectionToLauncherCommandTarget.GetSafeNormal();
		}
	}

	if (VehiclePawn->VehicleAimComp)
	{
		// AimComp에 저장된 최신 Weapon Aim Solution입니다.
		const FCFVehicleWeaponAimSolution WeaponAimSolution = VehiclePawn->VehicleAimComp->GetWeaponAimSolution();
		if (WeaponAimSolution.bHasValidSolution && !WeaponAimSolution.DesiredAimDirection.IsNearlyZero())
		{
			return WeaponAimSolution.DesiredAimDirection.GetSafeNormal();
		}

		// AimComp가 보유한 현재 로컬 조준 상태입니다.
		const FCFVehicleLocalAimState LocalAimState = VehiclePawn->VehicleAimComp->GetLocalAimState();
		if (VehiclePawn->TurretYawPivotComp && !LocalAimState.LocalAimTargetLocation.IsNearlyZero())
		{
			// 터렛 Yaw 피벗 위치에서 조준 목표까지의 월드 방향입니다.
			const FVector DirectionToAimTarget = LocalAimState.LocalAimTargetLocation - VehiclePawn->TurretYawPivotComp->GetComponentLocation();
			if (!DirectionToAimTarget.IsNearlyZero())
			{
				return DirectionToAimTarget.GetSafeNormal();
			}
		}

		if (!LocalAimState.LocalAimDirection.IsNearlyZero())
		{
			return LocalAimState.LocalAimDirection.GetSafeNormal();
		}
	}

	if (VehiclePawn->VehicleCameraComp)
	{
		// 카메라 컴포넌트가 직접 제공하는 현재 조준 방향입니다.
		const FVector CameraAimDirection = VehiclePawn->VehicleCameraComp->GetCurrentAimDirection();
		if (!CameraAimDirection.IsNearlyZero())
		{
			return CameraAimDirection.GetSafeNormal();
		}
	}

	// AimComp와 CameraComp가 유효하지 않을 때 사용할 Actor 정면 방향입니다.
	const FVector ActorForwardDirection = VehiclePawn->GetActorForwardVector().GetSafeNormal();
	return ActorForwardDirection.IsNearlyZero() ? FVector::ForwardVector : ActorForwardDirection;
}

// 현재 Weapon turret state를 시각 Yaw/Pitch pivot에 적용합니다.
void UCFVehicleVisualComp::UpdateVehicleTurretAimVisuals(const float DeltaSeconds)
{
	// 터렛 상태와 시각 피벗을 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	if (!VehiclePawn->VehicleWeaponComp)
	{
		if (VehiclePawn->VehicleAimComp)
		{
			VehiclePawn->VehicleAimComp->SetWeaponAimSolution(FCFVehicleWeaponAimSolution());
		}
		return;
	}

	if (!VehiclePawn->TurretMountRootComp || !VehiclePawn->TurretYawPivotComp || !VehiclePawn->TurretPitchPivotComp)
	{
		VehiclePawn->VehicleWeaponComp->ResetTurretState();
		if (VehiclePawn->VehicleAimComp)
		{
			VehiclePawn->VehicleAimComp->SetWeaponAimSolution(FCFVehicleWeaponAimSolution());
		}
		return;
	}

	if (!VehiclePawn->bLastTurretVisualAttached)
	{
		VehiclePawn->VehicleWeaponComp->UpdateTurretState(DeltaSeconds, VehiclePawn->GetActorForwardVector(), FTransform::Identity, false);
		VehiclePawn->TurretYawPivotComp->SetRelativeRotation(FRotator::ZeroRotator);
		VehiclePawn->TurretPitchPivotComp->SetRelativeRotation(FRotator::ZeroRotator);
		if (VehiclePawn->VehicleAimComp)
		{
			VehiclePawn->VehicleAimComp->SetWeaponAimSolution(FCFVehicleWeaponAimSolution());
		}
		return;
	}

	// 터렛 조준 계산에 사용할 월드 방향입니다.
	const FVector TurretAimWorldDirection = ResolveTurretAimWorldDirection();

	// 터렛 로컬 각도 계산의 기준이 되는 터렛 장착 루트 Transform입니다.
	const FTransform TurretReferenceTransform = VehiclePawn->TurretMountRootComp->GetComponentTransform();

	// WeaponComp가 터렛 상태 갱신에 성공했는지 여부입니다.
	const bool bTurretStateUpdated = VehiclePawn->VehicleWeaponComp->UpdateTurretState(DeltaSeconds, TurretAimWorldDirection, TurretReferenceTransform, VehiclePawn->bLastTurretVisualAttached);
	if (!bTurretStateUpdated)
	{
		return;
	}

	// 시각 피벗에 적용할 최신 터렛 조준 추적 상태입니다.
	const FCFVehicleTurretState TurretState = VehiclePawn->VehicleWeaponComp->GetTurretState();

	// Yaw 피벗에 적용할 상대 회전입니다.
	const FRotator YawPivotRelativeRotation(0.0f, TurretState.CurrentYawDeg, 0.0f);

	// Pitch 피벗에 적용할 상대 회전입니다.
	const FRotator PitchPivotRelativeRotation(TurretState.CurrentPitchDeg, 0.0f, 0.0f);

	VehiclePawn->TurretYawPivotComp->SetRelativeRotation(YawPivotRelativeRotation);
	VehiclePawn->TurretPitchPivotComp->SetRelativeRotation(PitchPivotRelativeRotation);
	VehiclePawn->RefreshWeaponAimSolution();
}

// 로컬 Owner 표시 안정화 계층을 준비합니다.
bool UCFVehicleVisualComp::PrepareOwnerVisualStabilization()
{
	// Owner 표시 설정과 시각 계층을 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return false;
	}

	bOwnerVisualStabilizationReady = false;

	if (!VehiclePawn->bEnableOwnerVisualStabilization)
	{
		ResetOwnerVisualStabilization();
		return false;
	}

	if ((VehiclePawn->GetNetMode() == NM_DedicatedServer) || !VehiclePawn->IsLocallyControlled())
	{
		ResetOwnerVisualStabilization();
		return false;
	}

	if (!VehiclePawn->OwnerVisualRootComp)
	{
		return false;
	}

	// Owner 표시 루트의 부모가 될 차량 물리 루트 SkeletalMeshComponent입니다.
	USkeletalMeshComponent* VehicleMeshComponent = FindSkeletalMeshComponentByName(VehiclePawn, TEXT("VehicleMesh"));
	if (!VehicleMeshComponent)
	{
		VehicleMeshComponent = VehiclePawn->GetMesh();
	}

	if (!VehicleMeshComponent)
	{
		return false;
	}

	if (VehiclePawn->OwnerVisualRootComp->GetAttachParent() != VehicleMeshComponent)
	{
		VehiclePawn->OwnerVisualRootComp->AttachToComponent(VehicleMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	VehiclePawn->OwnerVisualRootComp->SetRelativeLocation(FVector::ZeroVector);
	VehiclePawn->OwnerVisualRootComp->SetRelativeRotation(FRotator::ZeroRotator);
	VehiclePawn->OwnerVisualRootComp->SetRelativeScale3D(FVector::OneVector);

	// Owner 표시 루트 아래로 묶을 차체/휠 표시 컴포넌트 이름 목록입니다.
	const TArray<FName> OwnerVisualComponentNames =
	{
		TEXT("SM_Body"),
		TEXT("Wheel_Anchor_FL"),
		TEXT("Wheel_Anchor_FR"),
		TEXT("Wheel_Anchor_RL"),
		TEXT("Wheel_Anchor_RR"),
		TEXT("Wheel_Mesh_FL"),
		TEXT("Wheel_Mesh_FR"),
		TEXT("Wheel_Mesh_RL"),
		TEXT("Wheel_Mesh_RR")
	};

	// Owner 표시 루트 아래로 이동한 컴포넌트 개수입니다.
	int32 AttachedVisualComponentCount = 0;
	for (const FName& OwnerVisualComponentName : OwnerVisualComponentNames)
	{
		// 현재 이름으로 fresh resolve한 표시 대상 SceneComponent입니다.
		USceneComponent* VisualComponent = FindSceneComponentByName(VehiclePawn, OwnerVisualComponentName);
		if (AttachOwnerVisualComponent(VisualComponent))
		{
			++AttachedVisualComponentCount;
		}
	}

	if (VehicleMeshComponent && VehiclePawn->bHideOwnerPhysicsMeshWhenStabilized)
	{
		VehicleMeshComponent->SetVisibility(false, false);
		VehicleMeshComponent->SetHiddenInGame(true, false);
		bOwnerVisualPhysicsMeshHidden = true;
	}
	else if (VehicleMeshComponent && bOwnerVisualPhysicsMeshHidden)
	{
		VehicleMeshComponent->SetVisibility(true, false);
		VehicleMeshComponent->SetHiddenInGame(false, false);
		bOwnerVisualPhysicsMeshHidden = false;
	}

	SmoothedOwnerVisualRotation = VehiclePawn->GetActorRotation();
	bHasSmoothedOwnerVisualRotation = true;
	bOwnerVisualStabilizationReady = AttachedVisualComponentCount > 0;
	return bOwnerVisualStabilizationReady;
}

// 지정 표시 컴포넌트를 Owner 표시 루트 아래로 안전하게 이동합니다.
bool UCFVehicleVisualComp::AttachOwnerVisualComponent(USceneComponent* VisualComponent)
{
	// Owner 표시 루트를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !VisualComponent || !VehiclePawn->OwnerVisualRootComp)
	{
		return false;
	}

	if ((VisualComponent == VehiclePawn->OwnerVisualRootComp) || (VisualComponent == VehiclePawn->GetRootComponent()) || (VisualComponent == VehiclePawn->GetMesh()))
	{
		return false;
	}

	if (!VisualComponent->IsRegistered())
	{
		return false;
	}

	if (!VisualComponent->IsAttachedTo(VehiclePawn->OwnerVisualRootComp))
	{
		VisualComponent->AttachToComponent(VehiclePawn->OwnerVisualRootComp, FAttachmentTransformRules::KeepWorldTransform);
	}

	return true;
}

// 로컬 Owner 표시 루트의 안정화 회전을 갱신합니다.
void UCFVehicleVisualComp::UpdateOwnerVisualStabilization(const float DeltaSeconds)
{
	// Owner 표시 설정과 런타임 상태를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	if (!VehiclePawn->bEnableOwnerVisualStabilization)
	{
		ResetOwnerVisualStabilization();
		return;
	}

	if ((VehiclePawn->GetNetMode() == NM_DedicatedServer) || !VehiclePawn->IsLocallyControlled())
	{
		ResetOwnerVisualStabilization();
		return;
	}

	if (!VehiclePawn->OwnerVisualRootComp)
	{
		return;
	}

	if (!bOwnerVisualStabilizationReady)
	{
		// 이번 Tick에서 Owner 표시 안정화 계층 준비에 성공했는지 여부입니다.
		const bool bPreparedOwnerVisualThisFrame = PrepareOwnerVisualStabilization();
		if (bPreparedOwnerVisualThisFrame && VehiclePawn->WheelSyncComp && VehiclePawn->bVehicleRuntimeReady)
		{
			VehiclePawn->PrepareWheelSync();
		}
	}

	if (!bOwnerVisualStabilizationReady)
	{
		return;
	}

	// 현재 물리 Actor 회전입니다.
	const FRotator CurrentActorRotation = VehiclePawn->GetActorRotation();
	if (!bHasSmoothedOwnerVisualRotation)
	{
		SmoothedOwnerVisualRotation = CurrentActorRotation;
		bHasSmoothedOwnerVisualRotation = true;
	}

	// 음수 Tick 간격을 방지한 표시 안정화 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// 표시 루트 회전 보간에 사용할 안전한 보간 속도입니다.
	const float SafeInterpSpeed = FMath::Max(VehiclePawn->OwnerVisualStabilizationInterpSpeed, 0.1f);

	// 현재 보간 속도로 Actor 회전을 따라간 후보 표시 회전입니다.
	const FRotator InterpolatedVisualRotation = FMath::RInterpTo(SmoothedOwnerVisualRotation, CurrentActorRotation, SafeDeltaSeconds, SafeInterpSpeed);

	// 축별 사용 여부를 반영한 표시 회전 후보입니다.
	FRotator DesiredVisualRotation = CurrentActorRotation;
	if (VehiclePawn->bOwnerVisualStabilizePitchRoll)
	{
		DesiredVisualRotation.Pitch = InterpolatedVisualRotation.Pitch;
		DesiredVisualRotation.Roll = InterpolatedVisualRotation.Roll;
	}
	if (VehiclePawn->bOwnerVisualStabilizeYaw)
	{
		DesiredVisualRotation.Yaw = InterpolatedVisualRotation.Yaw;
	}

	SmoothedOwnerVisualRotation = ClampOwnerVisualStabilizedRotation(CurrentActorRotation, DesiredVisualRotation);
	VehiclePawn->OwnerVisualRootComp->SetWorldRotation(SmoothedOwnerVisualRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

// Owner 표시 안정화 상태를 기본 상태로 되돌립니다.
void UCFVehicleVisualComp::ResetOwnerVisualStabilization()
{
	// Owner 표시 설정과 시각 계층을 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	if (VehiclePawn->OwnerVisualRootComp)
	{
		VehiclePawn->OwnerVisualRootComp->SetRelativeRotation(FRotator::ZeroRotator);
	}

	SmoothedOwnerVisualRotation = VehiclePawn->GetActorRotation();
	bHasSmoothedOwnerVisualRotation = false;
	bOwnerVisualStabilizationReady = false;

	// 물리 루트 렌더링 복구 대상 SkeletalMeshComponent입니다.
	USkeletalMeshComponent* VehicleMeshComponent = FindSkeletalMeshComponentByName(VehiclePawn, TEXT("VehicleMesh"));
	if (!VehicleMeshComponent)
	{
		VehicleMeshComponent = VehiclePawn->GetMesh();
	}

	if (VehicleMeshComponent && bOwnerVisualPhysicsMeshHidden)
	{
		VehicleMeshComponent->SetVisibility(true, false);
		VehicleMeshComponent->SetHiddenInGame(false, false);
		bOwnerVisualPhysicsMeshHidden = false;
	}
}

// Owner 표시 회전의 Actor 대비 최대 지연각을 제한합니다.
FRotator UCFVehicleVisualComp::ClampOwnerVisualStabilizedRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	// Owner 표시 설정을 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	if (!VehiclePawn)
	{
		return DesiredVisualRotation;
	}

	// 표시 루트가 Actor 회전에서 벗어날 수 있는 최대 축별 각도입니다.
	const float SafeMaxLagDeg = FMath::Max(VehiclePawn->OwnerVisualStabilizationMaxLagDeg, 0.0f);

	// Actor Pitch에서 표시 Pitch까지의 지연각입니다.
	const float PitchLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Pitch, DesiredVisualRotation.Pitch);

	// Actor Yaw에서 표시 Yaw까지의 지연각입니다.
	const float YawLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Yaw, DesiredVisualRotation.Yaw);

	// Actor Roll에서 표시 Roll까지의 지연각입니다.
	const float RollLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Roll, DesiredVisualRotation.Roll);

	// 최대 지연각으로 제한한 표시 Pitch입니다.
	const float ClampedPitchDeg = CurrentActorRotation.Pitch + FMath::Clamp(PitchLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// 최대 지연각으로 제한한 표시 Yaw입니다.
	const float ClampedYawDeg = CurrentActorRotation.Yaw + FMath::Clamp(YawLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// 최대 지연각으로 제한한 표시 Roll입니다.
	const float ClampedRollDeg = CurrentActorRotation.Roll + FMath::Clamp(RollLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// 정규화 전 최종 표시 회전입니다.
	FRotator ClampedVisualRotation(ClampedPitchDeg, ClampedYawDeg, ClampedRollDeg);
	ClampedVisualRotation.Normalize();
	return ClampedVisualRotation;
}

// 로컬 SM_Body 전용 표시 안정화를 갱신합니다.
void UCFVehicleVisualComp::UpdateOwnerBodyVisualStabilization(const float DeltaSeconds)
{
	// Owner Body 표시 설정과 상태를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	if (!VehiclePawn->bEnableOwnerBodyVisualStabilization || VehiclePawn->bEnableOwnerVisualStabilization)
	{
		ResetOwnerBodyVisualStabilization();
		return;
	}

	if ((VehiclePawn->GetNetMode() == NM_DedicatedServer) || !VehiclePawn->IsLocallyControlled())
	{
		ResetOwnerBodyVisualStabilization();
		return;
	}

	// 안정화 대상 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("SM_Body"));
	if (!BodyMeshComponent || !BodyMeshComponent->IsRegistered() || BodyMeshComponent->IsSimulatingPhysics())
	{
		bOwnerBodyVisualStabilizationReady = false;
		return;
	}

	if (!bHasOriginalOwnerBodyVisualRelativeRotation)
	{
		// 안정화 전 차체 표시 기본 상대 회전입니다.
		const FRotator CurrentBodyRelativeRotation = BodyMeshComponent->GetRelativeRotation();
		OriginalOwnerBodyVisualRelativeRotation = CurrentBodyRelativeRotation;
		bHasOriginalOwnerBodyVisualRelativeRotation = true;
	}

	// 현재 물리 Actor 회전입니다.
	const FRotator CurrentActorRotation = VehiclePawn->GetActorRotation();
	if (!bHasSmoothedOwnerBodyVisualRotation)
	{
		SmoothedOwnerBodyVisualRotation = CurrentActorRotation;
		bHasSmoothedOwnerBodyVisualRotation = true;
	}

	// 음수 Tick 간격을 방지한 차체 표시 안정화 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// 차체 표시 회전 보간에 사용할 안전한 보간 속도입니다.
	const float SafeInterpSpeed = FMath::Max(VehiclePawn->OwnerBodyVisualInterpSpeed, 0.1f);

	// 현재 보간 속도로 Actor 회전을 따라간 후보 차체 표시 회전입니다.
	const FRotator InterpolatedBodyRotation = FMath::RInterpTo(SmoothedOwnerBodyVisualRotation, CurrentActorRotation, SafeDeltaSeconds, SafeInterpSpeed);

	// 축별 사용 여부를 반영한 차체 표시 회전 후보입니다.
	FRotator DesiredBodyRotation = CurrentActorRotation;
	if (VehiclePawn->bOwnerBodyVisualStabilizePitchRoll)
	{
		DesiredBodyRotation.Pitch = InterpolatedBodyRotation.Pitch;
		DesiredBodyRotation.Roll = InterpolatedBodyRotation.Roll;
	}
	if (VehiclePawn->bOwnerBodyVisualStabilizeYaw)
	{
		DesiredBodyRotation.Yaw = InterpolatedBodyRotation.Yaw;
	}

	SmoothedOwnerBodyVisualRotation = ClampOwnerBodyVisualRotation(CurrentActorRotation, DesiredBodyRotation);
	BodyMeshComponent->SetWorldRotation(SmoothedOwnerBodyVisualRotation, false, nullptr, ETeleportType::TeleportPhysics);
	bOwnerBodyVisualStabilizationReady = true;
}

// SM_Body 전용 표시 안정화를 기본 상대 회전으로 되돌립니다.
void UCFVehicleVisualComp::ResetOwnerBodyVisualStabilization()
{
	// Owner Body 표시 설정과 상태를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	// 리셋 대상 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(VehiclePawn, TEXT("SM_Body"));
	if (BodyMeshComponent && BodyMeshComponent->IsRegistered() && bHasOriginalOwnerBodyVisualRelativeRotation)
	{
		BodyMeshComponent->SetRelativeRotation(OriginalOwnerBodyVisualRelativeRotation);
	}

	SmoothedOwnerBodyVisualRotation = VehiclePawn->GetActorRotation();
	bHasSmoothedOwnerBodyVisualRotation = false;
	bOwnerBodyVisualStabilizationReady = false;
}

// SM_Body 표시 회전의 Actor 대비 최대 지연각을 제한합니다.
FRotator UCFVehicleVisualComp::ClampOwnerBodyVisualRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	// Owner Body 표시 설정을 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	if (!VehiclePawn)
	{
		return DesiredVisualRotation;
	}

	// 차체 표시가 Actor 회전에서 벗어날 수 있는 최대 축별 각도입니다.
	const float SafeMaxLagDeg = FMath::Max(VehiclePawn->OwnerBodyVisualMaxLagDeg, 0.0f);

	// Actor Pitch에서 차체 표시 Pitch까지의 지연각입니다.
	const float PitchLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Pitch, DesiredVisualRotation.Pitch);

	// Actor Yaw에서 차체 표시 Yaw까지의 지연각입니다.
	const float YawLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Yaw, DesiredVisualRotation.Yaw);

	// Actor Roll에서 차체 표시 Roll까지의 지연각입니다.
	const float RollLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Roll, DesiredVisualRotation.Roll);

	// 최대 지연각으로 제한한 차체 표시 Pitch입니다.
	const float ClampedPitchDeg = CurrentActorRotation.Pitch + FMath::Clamp(PitchLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// 최대 지연각으로 제한한 차체 표시 Yaw입니다.
	const float ClampedYawDeg = CurrentActorRotation.Yaw + FMath::Clamp(YawLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// 최대 지연각으로 제한한 차체 표시 Roll입니다.
	const float ClampedRollDeg = CurrentActorRotation.Roll + FMath::Clamp(RollLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// 정규화 전 최종 차체 표시 회전입니다.
	FRotator ClampedBodyRotation(ClampedPitchDeg, ClampedYawDeg, ClampedRollDeg);
	ClampedBodyRotation.Normalize();
	return ClampedBodyRotation;
}
