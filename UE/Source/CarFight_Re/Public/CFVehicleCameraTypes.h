// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.3.0
// Date: 2026-09-29
// Description: VCFX-P0-03 자유조준 대응 Lateral Presentation modulation runtime diagnostics 계약 추가
// Changelog:
// - v0.3.0: 실제 차체 Roll/RollRate/YawRate, body-motion intensity와 signed ViewAlignment runtime diagnostics 추가.
// - v0.2.0: bAimPresentation, FCFVehicleCameraGameplayView, normalized Driving FX 런타임 진단 필드를 additive 추가.
// - v0.1.4: Camera Aim Trace가 선택한 Actor를 런타임 상태에 추가.
// - v0.1.3: Camera Aim Trace 표면 선택 Hit을 AimBlocked와 분리하기 위해 bAimTraceHasBlockingHit을 추가.
// Migration:
// - AimTraceHitActor는 현재 프레임의 목표 표면 식별용이며, 발사 대상 잠금이나 소유권을 의미하지 않는다.
// - Camera Aim Trace Hit은 목표 표면 선택 결과이며, 총구 장애물 판정은 Weapon Aim Solution의 MuzzleBlocked를 사용한다.
// Scope: 카메라 모드, Aim Profile, 모드 플래그, 런타임 스냅샷을 공통으로 정의합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleCameraTypes.generated.h"

class AActor;

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

	// [v0.2.0] 외부 시스템이 명시적으로 전달하는 정밀 조준 Presentation 감쇠 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="조준 카메라 연출 감쇠 (bAimPresentation)", ToolTip="명시적인 조준/정밀 시점에서 Driving Camera FX를 감쇠합니다. AimProfileOverride나 무기 장착 여부에서 자동 추정하지 않습니다."))
	bool bAimPresentation = false;

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

/** 신규 Presentation FX에 오염되지 않은 Gameplay용 카메라 관측값입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleCameraGameplayView
{
	GENERATED_BODY()

	// 현재 Gameplay View가 유효하게 계산되었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Gameplay View", meta=(DisplayName="Gameplay View 유효 여부 (bValid)"))
	bool bValid = false;

	// SpringArm Collision까지 반영된 실제 Camera 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Gameplay View", meta=(DisplayName="Gameplay 시점 원점 (ViewOrigin)"))
	FVector ViewOrigin = FVector::ZeroVector;

	// 신규 Driving Roll 적용 직전 실제 Camera Forward 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Gameplay View", meta=(DisplayName="Gameplay 시점 방향 (ViewDirection)"))
	FVector ViewDirection = FVector::ForwardVector;

	// 신규 Driving Roll 적용 직전 실제 Camera Up 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Gameplay View", meta=(DisplayName="Gameplay 시점 위 방향 (ViewUpDirection)"))
	FVector ViewUpDirection = FVector::UpVector;

	// 신규 Speed FOV를 제외한 현재 Gameplay 수직 FOV입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Gameplay View", meta=(DisplayName="Gameplay 수직 FOV (VerticalFOVDeg)"))
	float VerticalFOVDeg = 0.0f;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Pitch/Roll 카메라 분리 적용 여부 (bCameraPitchRollIsolationApplied)", ToolTip="현재 프레임 카메라 계산에서 차량 Root의 Pitch/Roll 영향을 분리했는지 여부입니다."))
	bool bCameraPitchRollIsolationApplied = false;

	// [v0.1.2] 이번 프레임 카메라 기준 Yaw 완충 적용 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="카메라 Yaw 완충 적용 여부 (bCameraYawDampingApplied)", ToolTip="현재 프레임 카메라 계산에서 차량 기준 Yaw 완충이 적용되었는지 여부입니다."))
	bool bCameraYawDampingApplied = false;

	// [v0.1.2] 차량 Actor 또는 Pivot에서 읽은 목표 카메라 기준 Yaw입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="목표 카메라 기준 Yaw (TargetCameraBaseYaw)", ToolTip="차량 Actor 또는 Pivot에서 읽은 목표 카메라 기준 Yaw(deg)입니다."))
	float TargetCameraBaseYaw = 0.0f;

	// [v0.1.2] Yaw 완충 후 카메라 회전에 실제 사용한 기준 Yaw입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="완충 카메라 기준 Yaw (SmoothedCameraBaseYaw)", ToolTip="Yaw 완충 후 카메라 회전에 실제 사용한 기준 Yaw(deg)입니다."))
	float SmoothedCameraBaseYaw = 0.0f;

	// [v0.1.2] 목표 차량 Yaw와 완충 카메라 Yaw 사이의 현재 차이입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="카메라 Yaw 지연각 (CameraYawDampingLagDeg)", ToolTip="목표 차량 Yaw와 완충 카메라 Yaw 사이의 현재 차이(deg)입니다."))
	float CameraYawDampingLagDeg = 0.0f;

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

	// [v0.1.3] Camera Aim Trace가 표면 선택용 Blocking Hit을 얻었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Trace Blocking Hit 여부 (bAimTraceHasBlockingHit)", ToolTip="카메라 Aim Trace가 목표 표면을 선택하기 위해 Blocking Hit을 얻었는지 여부입니다. 이 값만으로 조준 가림으로 처리하지 않습니다."))
	bool bAimTraceHasBlockingHit = false;

	// [v0.1.4] Camera Aim Trace가 현재 목표 표면으로 선택한 Actor입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Trace 적중 Actor (AimTraceHitActor)", ToolTip="카메라 Aim Trace가 현재 목표 표면으로 선택한 Actor입니다. 총구 Trace가 같은 목표를 적중했는지 판별할 때 사용합니다."))
	TObjectPtr<AActor> AimTraceHitActor = nullptr;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="현재 FOV (CurrentFOV)", ToolTip="보간 적용 후 실제 Presentation Camera에 반영된 현재 FOV입니다."))
	float CurrentFOV = 0.0f;

	// [v0.2.0] 신규 Speed FOV를 제외한 Gameplay 소비용 현재 FOV입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="현재 Gameplay FOV (CurrentGameplayFOV)"))
	float CurrentGameplayFOV = 0.0f;

	// [v0.2.0] 현재 차량 기준속도 대비 평면 속도 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="속도 비율 (SpeedRatio)"))
	float SpeedRatio = 0.0f;

	// [v0.2.0] 필터 적용 후 정규화 가속률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="가속률 (AccelerationRate)"))
	float AccelerationRate = 0.0f;

	// [v0.2.0] 필터 적용 후 정규화 제동률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="제동률 (BrakingRate)"))
	float BrakingRate = 0.0f;

	// [v0.2.0] 필터 적용 후 signed 정규화 횡가속률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="횡가속률 (LateralRate)"))
	float LateralRate = 0.0f;

	// [v0.2.0] 현재 Speed 계열 Driving FX 감쇠 배율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="속도 FX 배율 (ResolvedSpeedFXScale)"))
	float ResolvedSpeedFXScale = 1.0f;

	// [v0.2.0] 현재 Motion 계열 Driving FX 감쇠 배율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="운동 FX 배율 (ResolvedMotionFXScale)"))
	float ResolvedMotionFXScale = 1.0f;

	// [v0.3.0] 실제 Vehicle Mesh에서 읽은 현재 차체 Roll 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="차체 Roll 각도 (VehicleBodyRollDeg)"))
	float VehicleBodyRollDeg = 0.0f;

	// [v0.3.0] 실제 Vehicle Mesh local X축 기준 현재 차체 Roll 각속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="차체 Roll 각속도 (VehicleBodyRollRateDegPerSec)"))
	float VehicleBodyRollRateDegPerSec = 0.0f;

	// [v0.3.0] 실제 Vehicle Mesh local Z축 기준 현재 차체 Yaw 각속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="차체 Yaw 각속도 (VehicleBodyYawRateDegPerSec)"))
	float VehicleBodyYawRateDegPerSec = 0.0f;

	// [v0.3.0] 차체 Roll/각속도 중 가장 강한 값을 보간한 0~1 Lateral Motion 강도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="횡방향 차체 Motion 강도 (LateralBodyMotionIntensity)"))
	float LateralBodyMotionIntensity = 0.0f;

	// [v0.3.0] 차량 수평 전방과 현재 자유시점 수평 전방의 signed 정렬값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="횡방향 시점 정렬 (LateralViewAlignment)", ToolTip="정면 +1, 측면 0, 후면 -1입니다. 자유 조준 시 화면 기준 Lateral Roll 강도와 방향을 조절합니다."))
	float LateralViewAlignment = 0.0f;

	// [v0.2.0] 최종 적용된 Speed FOV 오프셋입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="속도 FOV 오프셋 (SpeedFOVOffsetDeg)"))
	float SpeedFOVOffsetDeg = 0.0f;

	// [v0.2.0] 최종 적용된 Speed Arm 오프셋입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="속도 Arm 오프셋 (SpeedArmOffsetCm)"))
	float SpeedArmOffsetCm = 0.0f;

	// [v0.2.0] 최종 적용된 가속 Rear Kick입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="가속 Arm 킥 (AccelerationArmKickCm)"))
	float AccelerationArmKickCm = 0.0f;

	// [v0.2.0] 최종 적용된 제동 Forward Kick입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="제동 Arm 킥 (BrakingArmKickCm)"))
	float BrakingArmKickCm = 0.0f;

	// [v0.2.0] 최종 적용된 횡가속 Presentation Roll입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="횡가속 Roll (LateralRollDeg)"))
	float LateralRollDeg = 0.0f;

	// [v0.2.0] normalized Driving FX 경로가 현재 활성인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="정규화 주행 FX 활성 (bNormalizedDrivingFXActive)"))
	bool bNormalizedDrivingFXActive = false;

	// [v0.2.0] 첫 프레임/비정상 DeltaTime/Teleport로 Motion history를 초기화했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime|Driving FX", meta=(DisplayName="운동 History Reset 여부 (bMotionHistoryResetThisFrame)"))
	bool bMotionHistoryResetThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Trace 적중 위치 (AimHitLocation)", ToolTip="현재 카메라 기준 Aim Trace가 충돌한 월드 위치입니다. 미적중 시 최대 거리 끝점을 사용합니다."))
	FVector AimHitLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Trace 거리 (AimTraceDistance)", ToolTip="현재 Aim Trace가 계산한 실제 거리입니다."))
	float AimTraceDistance = 0.0f;
};
