// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.0
// Date: 2026-07-28
// Description: CarFight 터렛 마운트 DataAsset
// Scope: 무기를 얹고 회전시키는 거치대의 고정 Base / Yaw / Pitch 시각 메쉬, 피벗 소켓, 회전 한계, 회전 속도 값을 제공합니다.
// Changelog:
// - v1.7.0: 가변 MuzzleSocketNames 배열, 전체 소켓 필수 정책과 기존 단일 MuzzleSocketName fallback 계약을 추가.
// - v1.6.0: 총구 바로 앞의 발사 안전 구간만 MuzzleBlocked로 검사하는 MuzzleClearanceDistanceCm을 추가.
// - v1.5.0: 터렛별 정렬 중 발사 허용 정책 bAllowFireWhileAligning을 추가.
// - v1.4.0: MountProfile legacy 직접 TurretMountData 슬롯 제거에 맞춰 연결 기준을 EquipmentPresetData.DefaultTurretMountData로 갱신.
// - v1.3.0: 터렛 회전 제한이 MountProfile 교집합 없이 TurretMountData 단독 기준임을 명시.
// - v1.2.0: MuzzleSocketName을 실제 FireOrigin 전환 기준으로 승격하고 총구 소켓 축 기준을 명시.
// - v1.1.0: 고정 Base 메쉬와 YawPivot 소켓을 추가해 Base -> Yaw -> Pitch 3단 소켓 장착 구조를 지원.
// - v1.0.0: WeaponData와 분리된 최소 TurretMountData DataAsset 타입을 추가.
// Migration:
// - MuzzleSocketNames 배열이 비어 있으면 기존 MuzzleSocketName을 단일 총구 fallback으로 사용하므로 기존 DataAsset 재저장이 필요하지 않다.
// - MuzzleSocketNames 배열 순서는 SingleCycle 실제 발사 순서이며 None 항목은 bRequireAllMuzzles=false일 때 건너뛴다.
// - 기존 TurretMountData 인스턴스는 MuzzleClearanceDistanceCm=150cm 기본값을 사용하며, 0이면 총구 가림 사전 검사를 비활성화한다.
// - 기존 TurretMountData 인스턴스는 bAllowFireWhileAligning=true 기본값을 사용하며, false로 설정하면 터렛과 총구 정렬 완료 전 발사를 거부한다.
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

	// [v1.6.0] 디버그 패널에 발사 정책과 총구 안전 거리를 포함한 터렛 마운트 데이터 요약 문자열을 생성합니다.
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

		// [v1.7.0] Pitch 메쉬에서 SingleCycle이 순서대로 사용할 가변 총구 소켓 이름 배열입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Sockets", meta=(DisplayName="Muzzle 소켓 이름 배열 (MuzzleSocketNames)", ToolTip="다연장 런처가 순서대로 사용할 Pitch 메쉬 총구 소켓 이름 배열입니다. 배열 순서가 실제 발사 순서이며 각 소켓의 X축이 발사 방향이어야 합니다. 배열이 비어 있으면 기존 단일 MuzzleSocketName을 사용합니다."))
	TArray<FName> MuzzleSocketNames;

	// [v1.7.0] 배열에 지정한 모든 총구 소켓이 실제 Pitch 메쉬에 있어야 발사 준비로 인정할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Sockets", meta=(DisplayName="모든 Muzzle 소켓 필수 (Require All Muzzles)", ToolTip="True이면 MuzzleSocketNames 배열의 None 또는 누락 소켓이 하나라도 있을 때 다중 Muzzle 해결을 실패시킵니다. False이면 누락 항목을 건너뛰고 실제로 존재하는 소켓만 순환합니다."))
	bool bRequireAllMuzzles = false;

	// [v1.2.0] MuzzleSocketNames가 비어 있을 때 최종 FireOrigin에 사용할 기존 단일 총구 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Sockets", meta=(DisplayName="기존 단일 Muzzle 소켓 이름 (MuzzleSocketName)", ToolTip="MuzzleSocketNames 배열이 비어 있을 때 사용할 기존 단일 총구 소켓입니다. 소켓의 X축이 발사 방향이어야 하며 기존 DataAsset 호환을 위해 유지됩니다."))
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

	// [v1.5.0] 터렛 회전 또는 총구 정렬 중 현재 Muzzle 방향 발사를 허용할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|FirePolicy", meta=(DisplayName="정렬 중 발사 허용 (Allow Fire While Aligning)", ToolTip="True: 터렛 회전 또는 총구 정렬 중에도 현재 Muzzle 방향으로 발사합니다. False: 터렛과 총구 정렬이 완료될 때까지 발사를 거부합니다. MuzzleBlocked는 이 옵션과 관계없이 항상 발사를 거부합니다."))
	bool bAllowFireWhileAligning = true;

	// [v1.6.0] 총구에서 실제 발사체가 안전하게 빠져나갈 수 있는지 사전 검사할 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|FirePolicy", meta=(ClampMin="0.0", Units="cm", DisplayName="총구 안전 검사 거리 cm (MuzzleClearanceDistanceCm)", ToolTip="총구부터 이 거리 안의 비피해 장애물만 MuzzleBlocked로 처리합니다. 더 먼 장애물은 발사를 허용하고 실제 HitScan 또는 Projectile 충돌로 처리합니다. 0이면 사전 검사를 비활성화합니다."))
	float MuzzleClearanceDistanceCm = 150.0f;

	// [v1.0.0] 터렛 마운트가 차량 총중량에 더할 무게입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TurretMountData|Mass", meta=(ClampMin="0.0", DisplayName="터렛 마운트 중량 kg (TurretMountWeightKg)", ToolTip="이 터렛 마운트가 차량 총중량에 더하는 무게입니다. 현재 단계에서는 기록과 후속 중량 계산 후보입니다."))
	float TurretMountWeightKg = 350.0f;
};
