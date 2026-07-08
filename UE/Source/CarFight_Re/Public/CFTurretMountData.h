// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-07-02
// Description: CarFight 터렛 마운트 DataAsset
// Scope: 무기를 얹고 회전시키는 거치대의 고정 Base / Yaw / Pitch 시각 메쉬, 피벗 소켓, 회전 한계, 회전 속도 값을 제공합니다.
// Changelog:
// - v1.4.0: MountProfile legacy 직접 TurretMountData 슬롯 제거에 맞춰 연결 기준을 EquipmentPresetData.DefaultTurretMountData로 갱신.
// - v1.3.0: 터렛 회전 제한이 MountProfile 교집합 없이 TurretMountData 단독 기준임을 명시.
// - v1.2.0: MuzzleSocketName을 실제 FireOrigin 전환 기준으로 승격하고 총구 소켓 축 기준을 명시.
// - v1.1.0: 고정 Base 메쉬와 YawPivot 소켓을 추가해 Base -> Yaw -> Pitch 3단 소켓 장착 구조를 지원.
// - v1.0.0: WeaponData와 분리된 최소 TurretMountData DataAsset 타입을 추가.
// Migration:
// - EquipmentPresetData.DefaultTurretMountData에 이 DataAsset을 연결한다.
// - EquipmentPresetData 내부 TurretMountData가 비어 있으면 터렛 시각 / 조준 추적은 Missing으로 건너뛰고 MountProfile inline fallback은 사용하지 않는다.
// - TurretBaseMesh가 비어 있으면 YawPivot은 기존처럼 하드포인트 루트 기준으로 동작한다.
// - MuzzleSocketName이 Pitch 메쉬에 존재하면 최종 FireOrigin 위치와 방향은 해당 소켓을 우선 사용한다.
// - Min/Max Yaw/Pitch는 MountProfile 교집합 없이 런타임 터렛 회전 제한의 단일 기준이다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFTurretMountData.generated.h"

class UStaticMesh;

/**
 * 무기를 얹고 회전시키는 터렛 마운트 데이터입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFTurretMountData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 터렛 마운트 데이터 값을 초기화합니다.
	UCFTurretMountData();

	// [v1.1.0] 표시 가능한 터렛 시각 메쉬가 하나라도 지정되어 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TurretMountData", meta=(DisplayName="터렛 시각 메쉬 보유 여부 (Has Turret Visual Meshes)", ToolTip="Base, Yaw, Pitch 터렛 시각 메쉬가 하나라도 지정되어 있는지 반환합니다."))
	bool HasTurretVisualMeshes() const;

	// [v1.1.0] 디버그 패널에 표시할 터렛 마운트 데이터 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TurretMountData", meta=(DisplayName="터렛 마운트 요약 생성 (Build Turret Mount Summary)", ToolTip="디버그 패널과 로그에 표시할 터렛 마운트 핵심 데이터 요약 문자열을 생성합니다."))
	FString BuildTurretMountSummary() const;

	// [v1.0.0] 터렛 마운트 데이터를 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Identity", meta=(DisplayName="터렛 마운트 ID (TurretMountId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 터렛 마운트를 식별할 이름입니다. 예: Proto_RoofTurretMount"))
	FName TurretMountId = TEXT("Proto_RoofTurretMount");

	// [v1.1.0] 하드포인트에 고정되는 터렛 받침 메쉬입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Visual", meta=(DisplayName="터렛 Base 메쉬 (TurretBaseMesh)", ToolTip="차량 하드포인트에 고정되는 터렛 받침 메쉬입니다. 비어 있으면 Base 시각 표시를 생략하고 Yaw 회전부를 하드포인트 루트 기준으로 붙입니다."))
	TObjectPtr<UStaticMesh> TurretBaseMesh = nullptr;

	// [v1.1.0] Base 메쉬 또는 하드포인트 루트에서 Yaw 회전축 위치를 찾을 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Sockets", meta=(DisplayName="Yaw 피벗 소켓 이름 (YawPivotSocketName)", ToolTip="Base 메쉬에 만든 Yaw 회전부 장착 소켓 이름입니다. Base 메쉬가 비어 있거나 소켓이 없으면 하드포인트 루트 기준으로 Yaw 회전부를 붙입니다."))
	FName YawPivotSocketName = TEXT("YawPivot");

	// [v1.1.0] 하드포인트 루트에서 Base 메쉬에 적용할 보정 Transform입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Visual", meta=(DisplayName="터렛 Base 상대 Transform (TurretBaseRelativeTransform)", ToolTip="하드포인트 위치에 붙은 터렛 루트 기준 Base 메쉬의 상대 위치/회전/스케일 보정값입니다. Base 메쉬 피벗이 맞지 않을 때 조정합니다."))
	FTransform TurretBaseRelativeTransform = FTransform::Identity;

	// [v1.1.0] YawPivot 아래에서 좌우 Yaw 회전 기준이 될 터렛 상부 메쉬입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Visual", meta=(DisplayName="터렛 Yaw 메쉬 (TurretYawMesh)", ToolTip="YawPivot 아래에 붙어 좌우 회전할 터렛 상부 메쉬입니다. 비어 있으면 Yaw 시각 표시를 생략합니다."))
	TObjectPtr<UStaticMesh> TurretYawMesh = nullptr;

	// [v1.1.0] Yaw 회전부를 따라 좌우로 돌고 PitchPivot 기준으로 상하 회전할 상부 또는 포신 메쉬입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Visual", meta=(DisplayName="터렛 Pitch 메쉬 (TurretPitchMesh)", ToolTip="PitchPivot 아래에 붙어 Yaw를 따라 좌우 회전하고 Pitch로 상하 회전할 터렛 상부/포신 메쉬입니다. 비어 있으면 Pitch 시각 표시를 생략합니다."))
	TObjectPtr<UStaticMesh> TurretPitchMesh = nullptr;

	// [v1.1.0] YawPivot 기준으로 Yaw 메쉬에 적용할 보정 Transform입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Visual", meta=(DisplayName="터렛 Yaw 상대 Transform (TurretYawRelativeTransform)", ToolTip="YawPivot 기준 Yaw 메쉬의 상대 위치/회전/스케일 보정값입니다. 메쉬 피벗이 맞지 않을 때 조정합니다."))
	FTransform TurretYawRelativeTransform = FTransform::Identity;

	// [v1.1.0] Yaw 메쉬에서 Pitch 회전축 위치를 찾을 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Sockets", meta=(DisplayName="Pitch 피벗 소켓 이름 (PitchPivotSocketName)", ToolTip="Yaw 메쉬에 만든 Pitch 회전 기준 소켓 이름입니다. 없거나 비어 있으면 PitchPivot은 Yaw 메쉬 또는 YawPivot 원점에 붙습니다."))
	FName PitchPivotSocketName = TEXT("PitchPivot");

	// [v1.0.0] Pitch 피벗 소켓 또는 Yaw 메쉬 기준으로 Pitch 메쉬에 적용할 보정 Transform입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Visual", meta=(DisplayName="터렛 Pitch 상대 Transform (TurretPitchRelativeTransform)", ToolTip="PitchPivot 기준 Pitch 메쉬의 상대 위치/회전/스케일 보정값입니다. 메쉬마다 포신 피벗이 다를 때 조정합니다."))
	FTransform TurretPitchRelativeTransform = FTransform::Identity;

	// [v1.2.0] Pitch 메쉬에서 최종 FireOrigin 위치와 방향을 찾을 총구 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Sockets", meta=(DisplayName="Muzzle 소켓 이름 (MuzzleSocketName)", ToolTip="Pitch 메쉬에 만든 총구 소켓 이름입니다. 소켓이 있으면 FireOrigin 위치와 방향은 이 소켓을 우선 사용하며, 소켓의 X축이 발사 방향이어야 합니다."))
	FName MuzzleSocketName = TEXT("Muzzle");

	// [v1.0.0] 터렛 마운트 자체의 최소 좌우 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Rotation", meta=(DisplayName="최소 Yaw 각도 (MinYawDeg)", ToolTip="터렛 마운트 자체가 허용하는 최소 좌우 조준각입니다. 런타임 터렛 제한의 단일 기준입니다."))
	float MinYawDeg = -160.0f;

	// [v1.0.0] 터렛 마운트 자체의 최대 좌우 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Rotation", meta=(DisplayName="최대 Yaw 각도 (MaxYawDeg)", ToolTip="터렛 마운트 자체가 허용하는 최대 좌우 조준각입니다. 런타임 터렛 제한의 단일 기준입니다."))
	float MaxYawDeg = 160.0f;

	// [v1.0.0] 터렛 마운트 자체의 최소 상하 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Rotation", meta=(DisplayName="최소 Pitch 각도 (MinPitchDeg)", ToolTip="터렛 마운트 자체가 허용하는 최소 상하 조준각입니다. 런타임 터렛 제한의 단일 기준입니다."))
	float MinPitchDeg = -8.0f;

	// [v1.0.0] 터렛 마운트 자체의 최대 상하 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Rotation", meta=(DisplayName="최대 Pitch 각도 (MaxPitchDeg)", ToolTip="터렛 마운트 자체가 허용하는 최대 상하 조준각입니다. 런타임 터렛 제한의 단일 기준입니다."))
	float MaxPitchDeg = 25.0f;

	// [v1.0.0] 터렛 마운트가 좌우로 따라가는 최대 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Rotation", meta=(ClampMin="0.0", DisplayName="Yaw 회전 속도 (YawTurnRateDegPerSec)", ToolTip="터렛 마운트가 좌우 목표 각도를 따라가는 초당 최대 각도입니다."))
	float YawTurnRateDegPerSec = 35.0f;

	// [v1.0.0] 터렛 마운트가 상하로 따라가는 최대 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Rotation", meta=(ClampMin="0.0", DisplayName="Pitch 회전 속도 (PitchTurnRateDegPerSec)", ToolTip="터렛 마운트가 상하 목표 각도를 따라가는 초당 최대 각도입니다."))
	float PitchTurnRateDegPerSec = 20.0f;

	// [v1.0.0] 목표 각도에 도달했다고 볼 허용 오차입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Stabilization", meta=(ClampMin="0.0", DisplayName="안정화 허용 각도 (StabilizationToleranceDeg)", ToolTip="현재 각도와 목표 각도의 차이가 이 값 이하면 안정화 후보로 봅니다."))
	float StabilizationToleranceDeg = 2.0f;

	// [v1.0.0] 안정화 상태로 인정하기 위한 최소 유지 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Stabilization", meta=(ClampMin="0.0", DisplayName="안정화 시간 (AimSettleTimeSeconds)", ToolTip="목표 각도에 충분히 근접한 상태를 이 시간 이상 유지하면 안정화 상태로 봅니다."))
	float AimSettleTimeSeconds = 0.35f;

	// [v1.0.0] 터렛 마운트가 차량 총중량에 더할 무게입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Mass", meta=(ClampMin="0.0", DisplayName="터렛 마운트 중량 kg (TurretMountWeightKg)", ToolTip="이 터렛 마운트가 차량 총중량에 더하는 무게입니다. 현재 단계에서는 기록과 후속 중량 계산 후보입니다."))
	float TurretMountWeightKg = 350.0f;
};
