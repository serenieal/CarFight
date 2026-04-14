// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.1.0
// Date: 2026-04-10
// Description: CarFight 차량 카메라 공용 타입 정의
// Scope: 카메라 모드, Aim Profile, 모드 플래그, 런타임 스냅샷을 공통으로 정의합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleCameraTypes.generated.h"

/**
 * 차량 카메라의 현재 대표 모드입니다.
 * - Normal: 기본 주행/자유 조준 상태입니다.
 * - Combat: 전투 집중 상태입니다.
 * - Reverse: 후진 보조 상태입니다.
 * - Airborne: 공중 상태입니다.
 * - Destroyed: 차량 파괴 또는 행동 불능 상태입니다.
 * - Spectate: 관전자/리스폰 대기 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleCameraMode : uint8
{
	Normal UMETA(DisplayName="Normal"),
	Combat UMETA(DisplayName="Combat"),
	Reverse UMETA(DisplayName="Reverse"),
	Airborne UMETA(DisplayName="Airborne"),
	Destroyed UMETA(DisplayName="Destroyed"),
	Spectate UMETA(DisplayName="Spectate")
};

/**
 * 현재 차량 카메라에 적용할 모드 Modifier 플래그입니다.
 * - BaseMode 하나만으로 모든 상황을 표현하기보다, 외부 시스템이 간단히 상태를 전달할 수 있게 만듭니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleCameraModeFlags
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="전투 Modifier 사용 (bCombat)", ToolTip="True이면 전투 집중용 카메라 Modifier를 적용합니다."))
	bool bCombat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="후진 Modifier 사용 (bReverse)", ToolTip="True이면 후진 보조용 카메라 Modifier를 적용합니다."))
	bool bReverse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="공중 Modifier 사용 (bAirborne)", ToolTip="True이면 공중 상태용 카메라 Modifier를 적용합니다."))
	bool bAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="파괴 상태 사용 (bDestroyed)", ToolTip="True이면 파괴 상태 카메라를 우선 적용합니다."))
	bool bDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="관전자 상태 사용 (bSpectate)", ToolTip="True이면 관전자/리스폰 대기 카메라를 우선 적용합니다."))
	bool bSpectate = false;
};

/**
 * 현재 활성 무기 그룹 또는 임시 규칙이 제공하는 조준 가능 범위 프로필입니다.
 * - 카메라가 어디까지 회전 가능한지 결정합니다.
 * - 후속 단계에서는 무기 DataAsset이나 Weapon 시스템이 이 구조를 공급하는 것을 목표로 합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleCameraAimProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="프로필 이름 (ProfileName)", ToolTip="현재 Aim Profile을 식별하기 위한 이름입니다."))
	FName ProfileName = TEXT("DefaultAimProfile");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="최소 Yaw 각도 (MinYawDeg)", ToolTip="차량 정면 기준 최소 좌우 회전각(deg)입니다. 음수는 왼쪽을 뜻합니다."))
	float MinYawDeg = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="최대 Yaw 각도 (MaxYawDeg)", ToolTip="차량 정면 기준 최대 좌우 회전각(deg)입니다. 양수는 오른쪽을 뜻합니다."))
	float MaxYawDeg = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="최소 Pitch 각도 (MinPitchDeg)", ToolTip="차량 기준 최소 상하 회전각(deg)입니다. 음수는 아래를 뜻합니다."))
	float MinPitchDeg = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="최대 Pitch 각도 (MaxPitchDeg)", ToolTip="차량 기준 최대 상하 회전각(deg)입니다. 양수는 위를 뜻합니다."))
	float MaxPitchDeg = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Soft Yaw 한계 구간 (YawSoftLimitZoneDeg)", ToolTip="Yaw 하드 리미트에 가까워질 때 HUD 경고에 사용할 완충 구간(deg)입니다."))
	float YawSoftLimitZoneDeg = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Soft Pitch 한계 구간 (PitchSoftLimitZoneDeg)", ToolTip="Pitch 하드 리미트에 가까워질 때 HUD 경고에 사용할 완충 구간(deg)입니다."))
	float PitchSoftLimitZoneDeg = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="후방 보기 허용 (bAllowRearView)", ToolTip="True이면 이 Aim Profile이 사실상 후방 방향 시야를 허용하는 것으로 해석합니다."))
	bool bAllowRearView = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="타겟 프레이밍 사용 (bUseTargetFraming)", ToolTip="True이면 후속 타겟 유지 카메라에서 타겟 프레이밍 보정을 사용할 수 있습니다."))
	bool bUseTargetFraming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="추가 FOV 오프셋 (FOVOffset)", ToolTip="현재 Aim Profile이 요구하는 추가 FOV 보정값입니다."))
	float FOVOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="추가 Arm 길이 오프셋 (ArmLengthOffset)", ToolTip="현재 Aim Profile이 요구하는 카메라 Arm 길이 보정값입니다."))
	float ArmLengthOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="추가 높이 오프셋 (HeightOffset)", ToolTip="현재 Aim Profile이 요구하는 카메라 높이 보정값입니다."))
	float HeightOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="추가 좌우 오프셋 (SideOffset)", ToolTip="현재 Aim Profile이 요구하는 카메라 좌우 프레이밍 보정값입니다. 양수는 오른쪽입니다."))
	float SideOffset = 0.0f;
};

/**
 * 차량 카메라가 HUD와 디버그에 노출할 런타임 스냅샷입니다.
 * - 현재 모드
 * - 조준 각도
 * - Arm/FOV
 * - Aim Trace 상태
 * 를 한 번에 확인할 수 있도록 묶습니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleCameraRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="현재 카메라 모드 (CurrentCameraMode)", ToolTip="현재 프레임에 적용된 차량 카메라 대표 모드입니다."))
	ECFVehicleCameraMode CurrentCameraMode = ECFVehicleCameraMode::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="이전 카메라 모드 (PreviousCameraMode)", ToolTip="직전 프레임의 차량 카메라 대표 모드입니다."))
	ECFVehicleCameraMode PreviousCameraMode = ECFVehicleCameraMode::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="이번 프레임 모드 변경 여부 (bCameraModeChangedThisFrame)", ToolTip="이번 프레임에 카메라 모드가 변경되었는지 여부입니다."))
	bool bCameraModeChangedThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="활성 Aim Profile 이름 (ActiveAimProfileName)", ToolTip="현재 해석된 Aim Profile 이름입니다."))
	FName ActiveAimProfileName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="누적 Aim Yaw (AccumulatedAimYaw)", ToolTip="입력 누적 전후를 포함한 현재 Aim Yaw 누적값(deg)입니다."))
	float AccumulatedAimYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="누적 Aim Pitch (AccumulatedAimPitch)", ToolTip="입력 누적 전후를 포함한 현재 Aim Pitch 누적값(deg)입니다."))
	float AccumulatedAimPitch = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Clamp 적용 Aim Yaw (ClampedAimYaw)", ToolTip="Aim Profile의 제한각을 적용한 최종 Aim Yaw(deg)입니다."))
	float ClampedAimYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Clamp 적용 Aim Pitch (ClampedAimPitch)", ToolTip="Aim Profile의 제한각을 적용한 최종 Aim Pitch(deg)입니다."))
	float ClampedAimPitch = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Yaw 한계 근접 여부 (bAimAtYawLimit)", ToolTip="현재 Aim Yaw가 하드 리미트에 닿았거나 매우 가까운지 여부입니다."))
	bool bAimAtYawLimit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Pitch 한계 근접 여부 (bAimAtPitchLimit)", ToolTip="현재 Aim Pitch가 하드 리미트에 닿았거나 매우 가까운지 여부입니다."))
	bool bAimAtPitchLimit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="조준 가림 여부 (bAimBlocked)", ToolTip="현재 카메라 시선 또는 Aim Trace가 즉시 장애물에 막혀 있는지 여부입니다."))
	bool bAimBlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="현재 Aim 사격 가능 여부 (bWeaponCanFireAtCurrentAim)", ToolTip="현재 Aim 상태가 사격 가능으로 해석되는지 여부입니다. 1차 구현에서는 제한각 내 여부와 가림 상태를 기준으로 계산합니다."))
	bool bWeaponCanFireAtCurrentAim = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="목표 Arm 길이 (DesiredArmLength)", ToolTip="충돌 보정 전 목표 카메라 Arm 길이입니다."))
	float DesiredArmLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="해결된 Arm 길이 (SolvedArmLength)", ToolTip="충돌 보정 이후 실제 카메라와 Pivot 사이 거리입니다."))
	float SolvedArmLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="현재 Arm 길이 (CurrentArmLength)", ToolTip="현재 프레임 보간 적용 후 카메라 Arm 길이입니다."))
	float CurrentArmLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="목표 FOV (DesiredFOV)", ToolTip="현재 모드와 속도, Aim Profile을 반영한 목표 FOV입니다."))
	float DesiredFOV = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="현재 FOV (CurrentFOV)", ToolTip="보간 적용 후 실제 카메라에 반영된 현재 FOV입니다."))
	float CurrentFOV = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Trace 적중 위치 (AimHitLocation)", ToolTip="현재 카메라 기준 Aim Trace가 충돌한 월드 위치입니다. 미적중 시 최대 거리 끝점을 사용합니다."))
	FVector AimHitLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Trace 거리 (AimTraceDistance)", ToolTip="현재 Aim Trace가 계산한 실제 거리입니다."))
	float AimTraceDistance = 0.0f;
};
