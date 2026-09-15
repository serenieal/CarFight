// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-14
// Description: CarFight VehicleDebug reflected type declaration owner
// Changelog:
// - v1.0.0: CF-FQ-048 VPS-P0-04에서 CFVehiclePawn.h가 소유하던 VehicleDebug enum/struct 선언을 독립 Public type header로 이동.
// Migration:
// - 기존 reflected 이름, 필드 이름/타입, Blueprint 노출, UPROPERTY 메타데이터와 기본값 의미는 변경하지 않습니다. Pawn Debug facade와 RuntimeApply readback은 기존 경로를 계속 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleInputTypes.h"
#include "CFVehicleDriveComp.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleCameraTypes.h"
#include "CFDamageTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "CFTargetSelectTypes.h"
#include "CFVehicleDebugTypes.generated.h"

class UCFEquipmentPresetData;
class UCFTurretMountData;
class UCFWeaponData;
class UCFProjectileData;
class UCFDamageData;

/**
 * 화면 디버그에 표시할 Drive 상태 문자열 포맷 모드입니다.
 * - Off: 화면 디버그를 표시하지 않습니다.
 * - SingleLine: 한 줄 요약 문자열을 표시합니다.
 * - MultiLine: 줄바꿈이 포함된 멀티라인 문자열을 표시합니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleDebugDisplayMode : uint8
{
	Off UMETA(DisplayName="Off"),
	SingleLine UMETA(DisplayName="SingleLine"),
	MultiLine UMETA(DisplayName="MultiLine")
};

/**
 * VehicleDebug HUD용 핵심 요약 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugOverview
{
	GENERATED_BODY()

	// [v2.14.1] 현재 Pawn 런타임 초기화 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	// [v2.14.1] HUD 요약용 현재 Drive 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="HUD 요약에 사용할 현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	// [v2.14.1] HUD 요약용 현재 전체 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="현재 속도 km/h (SpeedKmh)", ToolTip="HUD 요약에 사용할 현재 차량 전체 속도 km/h 값입니다."))
	float SpeedKmh = 0.0f;

	// [v2.14.1] HUD 요약용 현재 전후 방향 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="전후 방향 속도 km/h (ForwardSpeedKmh)", ToolTip="HUD 요약에 사용할 현재 차량 전후 방향 속도 km/h 값입니다."))
	float ForwardSpeedKmh = 0.0f;

	// [v2.14.1] HUD 요약용 현재 입력 장치 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="입력 장치 모드 (DeviceMode)", ToolTip="HUD 요약에 사용할 현재 차량 입력 장치 모드입니다."))
	ECFVehicleInputDeviceMode DeviceMode = ECFVehicleInputDeviceMode::Auto;

	// [v2.14.1] HUD 요약용 현재 입력 주도권입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="입력 주도권 (InputOwner)", ToolTip="HUD 요약에 사용할 현재 차량 입력 주도권입니다."))
	ECFVehicleInputOwnership InputOwner = ECFVehicleInputOwnership::None;

	// [v2.14.1] HUD 요약용 마지막 상태 전이 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="마지막 전이 요약 (LastTransitionShortText)", ToolTip="HUD 요약에 사용할 마지막 Drive 상태 전이 요약 문자열입니다."))
	FString LastTransitionShortText = TEXT("DriveStateTransition: None");
};

/**
 * VehicleDebug 주행 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugDrive
{
	GENERATED_BODY()

	// [v2.14.1] 현재 Drive 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	// [v2.14.1] 이전 프레임 Drive 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="이전 Drive 상태 (PreviousDriveState)", ToolTip="이전 프레임 기준 VehicleDriveComp의 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	// [v2.14.1] 이번 프레임 상태 전이 발생 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="이번 프레임 상태 변경 여부 (bDriveStateChangedThisFrame)", ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	// [v2.14.1] 현재 전체 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="현재 속도 km/h (SpeedKmh)", ToolTip="현재 차량 전체 속도 km/h 값입니다."))
	float SpeedKmh = 0.0f;

	// [v2.14.1] 현재 전후 방향 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="전후 방향 속도 km/h (ForwardSpeedKmh)", ToolTip="현재 차량 전후 방향 속도 km/h 값입니다."))
	float ForwardSpeedKmh = 0.0f;

	// [v2.14.1] 현재 스로틀 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="스로틀 입력 (Throttle)", ToolTip="현재 VehicleDriveComp에 적용된 스로틀 입력값입니다."))
	float Throttle = 0.0f;

	// [v2.14.1] 현재 브레이크 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="브레이크 입력 (Brake)", ToolTip="현재 VehicleDriveComp에 적용된 브레이크 입력값입니다."))
	float Brake = 0.0f;

	// [v2.14.1] 현재 조향 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="조향 입력 (Steering)", ToolTip="현재 VehicleDriveComp에 적용된 조향 입력값입니다."))
	float Steering = 0.0f;

	// [v2.14.1] 현재 핸드브레이크 입력 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="핸드브레이크 입력 (bHandbrake)", ToolTip="현재 VehicleDriveComp에 적용된 핸드브레이크 입력 상태입니다."))
	bool bHandbrake = false;

	// [v2.14.1] 마지막 Drive 상태 전이 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="상태 전이 요약 (DriveStateTransitionSummary)", ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString DriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	// [v2.14.1] Drive 컴포넌트 최신 스냅샷입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="Drive 상태 스냅샷 (DriveStateSnapshot)", ToolTip="VehicleDriveComp가 계산한 최신 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot DriveStateSnapshot;
};

/**
 * VehicleDebug 입력 해석 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugInput
{
	GENERATED_BODY()

	// [v2.14.1] 현재 입력 장치 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 장치 모드 (DeviceMode)", ToolTip="현재 차량 입력 장치 모드입니다."))
	ECFVehicleInputDeviceMode DeviceMode = ECFVehicleInputDeviceMode::Auto;

	// [v2.14.1] 현재 입력 주도권입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 주도권 (InputOwner)", ToolTip="현재 차량 입력 적용의 주도권을 가진 경로입니다."))
	ECFVehicleInputOwnership InputOwner = ECFVehicleInputOwnership::None;

	// [v2.14.1] 현재 해석된 이동 입력 영역입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="해석 영역 (MoveZone)", ToolTip="현재 차량 2D 이동 입력이 해석된 영역입니다."))
	ECFVehicleMoveZone MoveZone = ECFVehicleMoveZone::None;

	// [v2.14.1] 현재 해석된 진행 방향 의도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="진행 방향 의도 (MoveIntent)", ToolTip="현재 차량 2D 이동 입력이 해석한 진행 방향 의도입니다."))
	ECFVehicleMoveDirectionIntent MoveIntent = ECFVehicleMoveDirectionIntent::None;

	// [v2.14.1] 원본 2D 이동 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="원본 이동 입력 (MoveRaw)", ToolTip="차량 이동 Input Action에서 들어온 최신 원본 2D 입력 벡터입니다."))
	FVector2D MoveRaw = FVector2D::ZeroVector;

	// [v2.14.1] 현재 이동 입력 강도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 강도 (MoveMagnitude)", ToolTip="차량 이동 입력 벡터의 최신 강도입니다."))
	float MoveMagnitude = 0.0f;

	// [v2.14.1] 현재 이동 입력 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 각도 (MoveAngle)", ToolTip="위가 0도, 시계 방향 증가 기준의 차량 이동 입력 각도입니다."))
	float MoveAngle = 0.0f;

	// [v2.14.1] 검은 영역 방향 유지 정책 적용 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="검은 영역 유지 사용 여부 (bUsedBlackZoneHold)", ToolTip="현재 프레임에 검은 영역 방향 유지 정책이 적용되었는지 여부입니다."))
	bool bUsedBlackZoneHold = false;

	// [v2.8.0] 게임패드 2D 이동 입력 방향에서 계산한 목표 조향값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float TargetSteeringInput = 0.0f;

	// [v2.8.0] 실제 DriveComp에 전달 중인 제한 속도 적용 조향값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float CurrentSteeringInput = 0.0f;

	// [v2.8.0] 목표 조향을 따라갈 때 사용한 마지막 조향 변화 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float LastSteeringTurnRate = 0.0f;

	// [v2.8.0] 중립 복귀 중 사용한 마지막 속도 기반 조향 복귀 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float LastSteeringReturnRate = 0.0f;

	// [v2.8.0] 현재 실제 조향값이 중립 0을 향해 복귀 중인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	bool bSteeringReturningToCenter = false;
};

/**
 * VehicleDebug 런타임 진단 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugRuntime
{
	GENERATED_BODY()

	// [v2.14.1] 현재 Pawn 런타임 초기화 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	// [v2.14.1] Drive 컴포넌트 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="Drive 컴포넌트 보유 여부 (bHasDriveComponent)", ToolTip="현재 VehicleDriveComp가 유효한지 여부입니다."))
	bool bHasDriveComponent = false;

	// [v2.14.1] WheelSync 컴포넌트 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="WheelSync 컴포넌트 보유 여부 (bHasWheelSyncComponent)", ToolTip="현재 WheelSyncComp가 유효한지 여부입니다."))
	bool bHasWheelSyncComponent = false;

	// [v2.14.1] 마지막 런타임 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="런타임 요약 (RuntimeSummary)", ToolTip="마지막 차량 런타임 초기화/적용 요약 문자열입니다."))
	FString RuntimeSummary = TEXT("NotInitialized");

	// [v2.14.1] 마지막 초기화 시도 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="마지막 초기화 시도 요약 (LastInitAttemptSummary)", ToolTip="최근 차량 런타임 초기화 시도 기준 요약 문자열입니다."))
	FString LastInitAttemptSummary = TEXT("NotInitialized");

	// [v2.14.1] 마지막 검증 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="마지막 검증 요약 (LastValidationSummary)", ToolTip="최근 차량 런타임 검증 기준 요약 문자열입니다."))
	FString LastValidationSummary = TEXT("NotInitialized");
};

/**
 * VehicleDebug 카메라 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugCamera
{
	GENERATED_BODY()

	// [v2.7.0] VehicleCameraComp 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="VehicleCameraComp 보유 여부 (bHasVehicleCameraComponent)", ToolTip="현재 Pawn이 VehicleCameraComp를 보유하고 있는지 여부입니다."))
	bool bHasVehicleCameraComponent = false;

	// [v2.7.0] VehicleCameraComp가 제공하는 원본 카메라 런타임 스냅샷입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="카메라 런타임 상태 (CameraRuntimeState)", ToolTip="VehicleCameraComp가 계산한 현재 카메라 런타임 스냅샷입니다."))
	FCFVehicleCameraRuntimeState CameraRuntimeState;

	// [v2.7.0] 목표 Arm 길이 대비 실제 해결 Arm 길이 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="충돌 압축 비율 (CollisionCompressionRatio)", ToolTip="DesiredArmLength 대비 SolvedArmLength 비율입니다. 1에 가까울수록 압축이 적고, 0에 가까울수록 크게 눌린 상태입니다."))
	float CollisionCompressionRatio = 1.0f;

	// [v2.7.0] 카메라가 충돌 등으로 의미 있게 압축된 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="충돌 압축 상태 (bCameraCompressedByCollision)", ToolTip="True이면 현재 카메라가 목표 Arm 길이보다 의미 있게 짧아진 상태입니다."))
	bool bCameraCompressedByCollision = false;
};

/**
 * VehicleDebug 조준 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugAim
{
	GENERATED_BODY()

	// [v2.16.0] VehicleAimComp 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="VehicleAimComp 보유 여부 (bHasVehicleAimComponent)", ToolTip="현재 Pawn이 VehicleAimComp를 보유하고 있는지 여부입니다."))
	bool bHasVehicleAimComponent = false;

	// [v2.16.0] Aim 런타임 준비 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Aim 런타임 준비 완료 여부 (bAimRuntimeReady)", ToolTip="VehicleAimComp가 Owner Pawn과 VehicleCameraComp 참조를 준비했는지 여부입니다."))
	bool bAimRuntimeReady = false;

	// [v2.64.0] 로컬 플레이어 기준 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="로컬 Aim 상태 (LocalAimState)", ToolTip="VehicleAimComp가 계산한 로컬 플레이어 기준 Aim 상태입니다."))
	FCFVehicleLocalAimState LocalAimState;

	// [v2.66.0] 로컬 발사 검증 기준 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="발사 검증 상태 (FireValidationState)", ToolTip="VehicleAimComp가 보유한 로컬 발사 검증 기준 Aim 상태입니다."))
	FCFVehicleFireValidationState FireValidationState;

	// [v2.66.0] 로컬 디버그와 발사 결과 표시용 Aim 시각 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Aim 시각 상태 (AimVisualState)", ToolTip="VehicleAimComp가 보유한 로컬 디버그와 발사 결과 표시용 Aim 시각 상태입니다."))
	FCFVehicleAimVisualState AimVisualState;

	// [v2.16.0] 로컬 Reticle 표시 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Reticle 상태 (ReticleState)", ToolTip="VehicleAimComp가 계산한 현재 로컬 Reticle 표시 상태입니다."))
	ECFVehicleReticleState ReticleState = ECFVehicleReticleState::Hidden;

	// [v2.16.0] Aim 런타임 초기화 또는 갱신 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Aim 런타임 요약 (AimRuntimeSummary)", ToolTip="VehicleAimComp의 마지막 런타임 초기화 또는 갱신 요약 문자열입니다."))
	FString AimRuntimeSummary = TEXT("AimRuntime: Missing");

	// [v2.64.0] 마지막 발사 명령 디버그 캐시입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="마지막 발사 명령 (LastFireRequest)", ToolTip="Pawn이 마지막으로 생성하거나 레거시 wrapper에서 받은 로컬 발사 명령 디버그 캐시입니다."))
	FCFVehicleFireRequest LastFireRequest;

	// [v2.64.0] 마지막 발사 결과 디버그 캐시입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="마지막 발사 결과 (LastFireResult)", ToolTip="로컬 발사 검증 뒤 Pawn이 마지막으로 적용한 발사 결과 디버그 캐시입니다."))
	FCFVehicleFireResult LastFireResult;
};

/**
 * VehicleDebug 선택 대상 상세 카테고리입니다.
 * 선택된 Actor의 TargetSelect 표시 정보와 방어·내구도 컴포넌트 상태를 읽기 전용으로 보관합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugTarget
{
	GENERATED_BODY()

	// [v2.138.0] 현재 Pawn이 TargetSelectComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="TargetSelectComp 보유 여부 (bHasTargetSelectComponent)", ToolTip="현재 Pawn이 선택 대상을 관리하는 TargetSelectComp를 보유하는지 표시합니다."))
	bool bHasTargetSelectComponent = false;

	// [v2.138.0] TargetSelectComp에 선택 대상 기록이 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 존재 여부 (bHasSelectedTarget)", ToolTip="현재 TargetSelectComp에 선택 대상 기록이 존재하는지 표시합니다."))
	bool bHasSelectedTarget = false;

	// [v2.138.0] 현재 선택 대상 Actor가 런타임에서 유효한지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 유효 여부 (bSelectedTargetValid)", ToolTip="현재 선택 대상 Actor가 제거되지 않았고 선택 가능한 유효 상태인지 표시합니다."))
	bool bSelectedTargetValid = false;

	// [v2.138.0] 현재 선택 대상 Actor의 런타임 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 Actor 이름 (SelectedTargetActorName)", ToolTip="현재 선택 대상 Actor 인스턴스의 런타임 이름입니다."))
	FString SelectedTargetActorName = TEXT("None");

	// [v2.138.0] 현재 선택 대상의 안정 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 ID (SelectedTargetId)", ToolTip="TargetSelect 표시 정보에서 읽은 현재 선택 대상의 안정 식별자입니다."))
	FName SelectedTargetId = NAME_None;

	// [v2.138.0] 현재 선택 대상의 사용자 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 표시 이름 (SelectedTargetDisplayName)", ToolTip="TargetSelect 표시 정보에서 읽은 현재 선택 대상의 사용자 표시 이름입니다."))
	FText SelectedTargetDisplayName;

	// [v2.138.0] 현재 선택 대상의 추적 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 추적 상태 (SelectedTargetTrackState)", ToolTip="현재 선택 대상이 가시, 가림, 추정 추적 또는 신호 손실 중인지 표시합니다."))
	ECFTargetTrackState SelectedTargetTrackState = ECFTargetTrackState::Invalid;

	// [v2.138.0] 선택 대상이 VehicleDefenseComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 VehicleDefenseComp 보유 여부 (bHasSelectedTargetDefenseComponent)", ToolTip="현재 선택 대상 Actor가 Shield와 방향별 Armor를 관리하는 VehicleDefenseComp를 보유하는지 표시합니다."))
	bool bHasSelectedTargetDefenseComponent = false;

	// [v2.138.0] 선택 대상의 DefenseData 초기화 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 방어 초기화 여부 (bSelectedTargetDefenseInitialized)", ToolTip="현재 선택 대상의 VehicleDefenseComp가 유효한 DefenseData로 초기화됐는지 표시합니다."))
	bool bSelectedTargetDefenseInitialized = false;

	// [v2.138.0] 선택 대상이 Legacy Integrity 직접 피해 경로를 사용하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 Legacy 방어 Fallback 여부 (bSelectedTargetUsingLegacyDefenseFallback)", ToolTip="True이면 선택 대상이 DefenseData 없이 기존 Integrity 직접 피해 경로를 사용합니다."))
	bool bSelectedTargetUsingLegacyDefenseFallback = false;

	// [v2.138.0] 선택 대상의 현재 Shield, 6방향 Armor와 재생 상태 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 방어 상태 요약 (SelectedTargetDefenseSummary)", ToolTip="선택 대상 VehicleDefenseComp의 현재 Shield, 방향별 Armor, 재생 여부와 남은 재생 지연을 읽기 전용으로 표시합니다."))
	FString SelectedTargetDefenseSummary = TEXT("선택 대상 VehicleDefenseComp 없음");

	// [v2.138.0] 선택 대상에 마지막 전체 방어 피해 결과가 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 마지막 방어 결과 존재 여부 (bHasSelectedTargetLastDamageResult)", ToolTip="선택 대상 VehicleDefenseComp에 마지막 Shield, Armor, 관통과 Integrity 피해 결과가 기록됐는지 표시합니다."))
	bool bHasSelectedTargetLastDamageResult = false;

	// [v2.138.0] 선택 대상의 마지막 전체 방어 피해 결과 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 마지막 방어 결과 요약 (SelectedTargetLastDamageResultSummary)", ToolTip="선택 대상에 마지막으로 적용된 방향, Shield 흡수, Armor 흡수·관통과 Integrity 결과를 표시합니다."))
	FString SelectedTargetLastDamageResultSummary = TEXT("선택 대상 방어 피해 기록 없음");

	// [v2.138.0] 선택 대상이 VehicleHealthComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 VehicleHealthComp 보유 여부 (bHasSelectedTargetHealthComponent)", ToolTip="현재 선택 대상 Actor가 차량 내구도를 관리하는 VehicleHealthComp를 보유하는지 표시합니다."))
	bool bHasSelectedTargetHealthComponent = false;

	// [v2.138.0] 선택 대상의 VehicleHealthComp 초기화 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 내구도 초기화 여부 (bSelectedTargetHealthInitialized)", ToolTip="현재 선택 대상의 최대·현재 Integrity가 초기화됐는지 표시합니다."))
	bool bSelectedTargetHealthInitialized = false;

	// [v2.138.0] 선택 대상의 현재 차량 내구도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 현재 Integrity (SelectedTargetCurrentIntegrity)", ToolTip="현재 선택 대상 VehicleHealthComp의 남은 Vehicle Integrity입니다."))
	float SelectedTargetCurrentIntegrity = 0.0f;

	// [v2.138.0] 선택 대상의 최대 차량 내구도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 최대 Integrity (SelectedTargetMaximumIntegrity)", ToolTip="현재 선택 대상 VehicleHealthComp의 최대 Vehicle Integrity입니다."))
	float SelectedTargetMaximumIntegrity = 0.0f;

	// [v2.138.0] 선택 대상이 파괴 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 파괴 여부 (bSelectedTargetDestroyed)", ToolTip="현재 선택 대상 VehicleHealthComp가 파괴 상태로 전환됐는지 표시합니다."))
	bool bSelectedTargetDestroyed = false;

	// [v2.138.0] 선택 대상 약한 참조와 해제·추적 상태 수명 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 수명 요약 (SelectedTargetLifetimeSummary)", ToolTip="TargetSelectComp가 기록한 선택 대상 유효성, 추적 상태와 마지막 해제 사유 요약입니다."))
	FString SelectedTargetLifetimeSummary = TEXT("SelectedTarget: None");
};

/**
 * VehicleDebug 무기 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugWeapon
{
	GENERATED_BODY()

	// [v2.76.0] VehicleWeaponComp 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="VehicleWeaponComp 보유 여부 (bHasVehicleWeaponComponent)", ToolTip="현재 Pawn이 VehicleWeaponComp를 보유하고 있는지 여부입니다."))
	bool bHasVehicleWeaponComponent = false;

	// [v2.76.0] Weapon 런타임 준비 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="Weapon 런타임 준비 완료 여부 (bWeaponRuntimeReady)", ToolTip="VehicleWeaponComp가 활성 장착 프로파일과 하드포인트 슬롯을 찾았는지 여부입니다."))
	bool bWeaponRuntimeReady = false;

	// [v2.76.0] WeaponComp가 우선 사용할 장착 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 장착 프로파일 ID (ActiveMountProfileId)", ToolTip="VehicleWeaponComp가 현재 우선 사용하는 장착 프로파일 ID입니다."))
	FName ActiveMountProfileId = NAME_None;

	// [v2.76.0] 마지막으로 계산된 실제 발사 원점입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 발사 원점 (LastFireOrigin)", ToolTip="VehicleWeaponComp가 마지막으로 계산한 실제 발사 위치와 방향입니다."))
	FCFVehicleFireOrigin LastFireOrigin;

	// [v2.76.0] WeaponComp의 마지막 런타임 초기화 또는 FireOrigin 계산 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="Weapon 런타임 요약 (LastWeaponRuntimeSummary)", ToolTip="VehicleWeaponComp의 마지막 런타임 초기화 또는 FireOrigin 계산 결과 요약 문자열입니다."))
	FString LastWeaponRuntimeSummary = TEXT("WeaponRuntime: Missing");

	// [v2.101.0] 현재 활성 장착 프로파일에서 해석한 EquipmentPresetData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData (ActiveEquipmentPresetData)", ToolTip="현재 활성 장착 프로파일에서 해석한 장비 프리셋 DataAsset입니다. 비어 있으면 장비 데이터는 Missing 상태입니다."))
	TObjectPtr<UCFEquipmentPresetData> ActiveEquipmentPresetData = nullptr;

	// [v2.100.0] 현재 활성 EquipmentPresetData가 지정되어 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData 지정 여부 (bActiveEquipmentPresetDataAssigned)", ToolTip="현재 활성 장착 프로파일에 장비 프리셋 DataAsset이 지정되어 있는지 여부입니다."))
	bool bActiveEquipmentPresetDataAssigned = false;

	// [v2.100.0] 현재 활성 EquipmentPresetData가 장착 프로파일과 호환되는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData 호환 여부 (bActiveEquipmentPresetDataCompatible)", ToolTip="현재 활성 EquipmentPresetData가 장착 타입과 크기 제한을 통과했는지 여부입니다."))
	bool bActiveEquipmentPresetDataCompatible = false;

	// [v2.100.0] 현재 활성 EquipmentPresetData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 장비 프리셋 ID (ActiveEquipmentPresetId)", ToolTip="현재 활성 EquipmentPresetData의 EquipmentId입니다. EquipmentPresetData가 비어 있으면 None입니다."))
	FName ActiveEquipmentPresetId = NAME_None;

	// [v2.100.0] 현재 활성 EquipmentPresetData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData 요약 (ActiveEquipmentPresetSummary)", ToolTip="현재 활성 EquipmentPresetData의 장비 조합 요약 문자열입니다."))
	FString ActiveEquipmentPresetSummary = TEXT("EquipmentPresetData: MissingOptional");

	// [v2.101.0] 현재 활성 EquipmentPresetData에서 해석한 TurretMountData이며, 비어 있으면 Missing 상태로 표시합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 TurretMountData (ActiveTurretMountData)", ToolTip="현재 활성 EquipmentPresetData에서 해석한 터렛 마운트 DataAsset입니다. 비어 있으면 터렛 시각 / 조준 추적은 Missing으로 표시되고 MountProfile 직접 fallback은 사용하지 않습니다."))
	TObjectPtr<UCFTurretMountData> ActiveTurretMountData = nullptr;

	// [v2.101.0] 현재 활성 TurretMountData가 EquipmentPresetData에서 해석되었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 TurretMountData 지정 여부 (bActiveTurretMountDataAssigned)", ToolTip="현재 활성 장착 프로파일에서 터렛 마운트 DataAsset이 EquipmentPresetData로 해석되었는지 여부입니다."))
	bool bActiveTurretMountDataAssigned = false;

	// [v2.87.0] 현재 활성 TurretMountData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 터렛 마운트 ID (ActiveTurretMountId)", ToolTip="현재 활성 TurretMountData의 TurretMountId입니다. TurretMountData가 비어 있으면 None입니다."))
	FName ActiveTurretMountId = NAME_None;

	// [v2.87.0] 현재 활성 TurretMountData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 터렛 마운트 요약 (ActiveTurretMountSummary)", ToolTip="현재 활성 TurretMountData의 시각 메쉬, 피벗 소켓, 회전 한계, 회전 속도 요약입니다."))
	FString ActiveTurretMountSummary = TEXT("TurretMountData: NotInitialized");

	// [v2.86.0] 현재 활성 장착 프로파일의 터렛 시각 메쉬가 차량에 붙었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 시각 장착 여부 (bTurretVisualAttached)", ToolTip="현재 활성 장착 프로파일의 터렛 시각 메쉬가 하드포인트 기준으로 차량에 붙었는지 여부입니다."))
	bool bTurretVisualAttached = false;

	// [v2.88.0] 현재 터렛 시각 장착 상태 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 시각 요약 (TurretVisualSummary)", ToolTip="터렛 Base/Yaw/Pitch 메쉬 연결, 하드포인트, 피벗 소켓 적용 상태를 요약한 문자열입니다."))
	FString TurretVisualSummary = TEXT("TurretVisual: NotInitialized");

	// [v2.89.0] 현재 터렛 조준 추적 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 조준 상태 (TurretState)", ToolTip="WeaponComp가 계산한 터렛 목표 Yaw/Pitch와 현재 추적 Yaw/Pitch 상태입니다."))
	FCFVehicleTurretState TurretState;

	// [v2.89.0] 현재 터렛 조준 추적 계산 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 런타임 요약 (TurretRuntimeSummary)", ToolTip="WeaponComp가 마지막으로 계산한 터렛 조준 추적 상태 요약입니다."))
	FString TurretRuntimeSummary = TEXT("TurretRuntime: NotInitialized");

	// [v2.88.0] 현재 터렛 Base 메쉬 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 Base 메쉬 이름 (TurretBaseMeshName)", ToolTip="현재 하드포인트에 고정된 터렛 받침/Base 메쉬 이름입니다. 비어 있으면 None입니다."))
	FName TurretBaseMeshName = NAME_None;

	// [v2.88.0] 현재 터렛 Yaw 메쉬 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 Yaw 메쉬 이름 (TurretYawMeshName)", ToolTip="현재 YawPivot 아래에 붙은 터렛 회전부/Yaw 메쉬 이름입니다. 비어 있으면 None입니다."))
	FName TurretYawMeshName = NAME_None;

	// [v2.88.0] 현재 터렛 Pitch 메쉬 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 Pitch 메쉬 이름 (TurretPitchMeshName)", ToolTip="현재 PitchPivot 아래에 붙은 터렛 상부/포신/Pitch 메쉬 이름입니다. 비어 있으면 None입니다."))
	FName TurretPitchMeshName = NAME_None;

	// [v2.77.0] 현재 활성 장착 프로파일에 연결된 WeaponData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 WeaponData (ActiveWeaponData)", ToolTip="현재 활성 장착 프로파일에 연결된 WeaponData입니다."))
	TObjectPtr<UCFWeaponData> ActiveWeaponData = nullptr;

	// [v2.77.0] 현재 활성 WeaponData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 ID (ActiveWeaponId)", ToolTip="현재 활성 WeaponData의 WeaponId입니다. WeaponData가 비어 있으면 None입니다."))
	FName ActiveWeaponId = NAME_None;

	// [v2.77.0] 현재 활성 WeaponData가 장착 프로파일과 호환되는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 WeaponData 호환 여부 (bActiveWeaponDataCompatible)", ToolTip="현재 활성 WeaponData가 장착 타입과 크기 제한을 통과했는지 여부입니다."))
	bool bActiveWeaponDataCompatible = false;

	// [v2.77.0] 현재 활성 WeaponData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 WeaponData 요약 (ActiveWeaponSummary)", ToolTip="현재 활성 WeaponData의 핵심 전투 데이터 요약 문자열입니다."))
	FString ActiveWeaponSummary = TEXT("WeaponData: MissingOptional");

	// [v2.79.0] 현재 활성 WeaponData에 연결된 ProjectileData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 ProjectileData (ActiveProjectileData)", ToolTip="현재 활성 WeaponData에 연결된 ProjectileData입니다. 비어 있으면 Dummy HitScan fallback을 유지합니다."))
	TObjectPtr<UCFProjectileData> ActiveProjectileData = nullptr;

	// [v2.79.0] 현재 활성 ProjectileData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 발사체 ID (ActiveProjectileId)", ToolTip="현재 활성 ProjectileData의 ProjectileId입니다. ProjectileData가 비어 있으면 None입니다."))
	FName ActiveProjectileId = NAME_None;

	// [v2.79.0] 현재 활성 ProjectileData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 ProjectileData 요약 (ActiveProjectileSummary)", ToolTip="현재 활성 ProjectileData의 핵심 발사체 데이터 요약 문자열입니다."))
	FString ActiveProjectileSummary = TEXT("ProjectileData: MissingOptional");

	// [v2.80.0] 현재 활성 ProjectileData가 Projectile Actor 전환 후보인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 Projectile 스폰 준비 여부 (bActiveProjectileSpawnReady)", ToolTip="활성 ProjectileData와 ProjectileActorClass가 모두 유효해 Projectile Actor Pool 확보 경로를 사용할 수 있는지 여부입니다."))
	bool bActiveProjectileSpawnReady = false;

	// [v2.80.0] 현재 활성 Projectile 실행 경로 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 Projectile 실행 요약 (ActiveProjectileExecutionSummary)", ToolTip="Dummy HitScan 유지 또는 Projectile 전환 준비 상태를 설명하는 요약 문자열입니다."))
	FString ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: DummyHitScanFallback");

	// [v2.96.0] 현재 활성 ProjectileData에서 해석한 DamageData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 DamageData (ActiveDamageData)", ToolTip="현재 활성 ProjectileData에서 해석한 DamageData입니다. 비어 있으면 ProjectileData.DamageProfileId fallback 또는 미지정 상태만 표시합니다."))
	TObjectPtr<UCFDamageData> ActiveDamageData = nullptr;

	// [v2.96.0] 현재 활성 DamageData의 DamageId 또는 ProjectileData fallback DamageProfileId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 피해 ID (ActiveDamageId)", ToolTip="현재 활성 DamageData의 DamageId 또는 fallback DamageProfileId입니다."))
	FName ActiveDamageId = NAME_None;

	// [v2.96.0] 현재 활성 DamageData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 DamageData 요약 (ActiveDamageSummary)", ToolTip="현재 활성 DamageData 또는 fallback DamageProfileId의 요약 문자열입니다."))
	FString ActiveDamageSummary = TEXT("DamageData: MissingOptional");

	// [v2.95.0] 현재 활성 DamageData가 어떤 경로로 해석되었는지 설명하는 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 DamageData 해석 요약 (ActiveDamageResolutionSummary)", ToolTip="ProjectileData.DefaultDamageData 또는 ProjectileData.DamageProfileId fallback 중 어떤 경로가 사용됐는지 설명합니다."))
	FString ActiveDamageResolutionSummary = TEXT("DamageResolution: MissingOptional");

	// [v2.97.0] 마지막 Damage HitContext 기록이 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 HitContext 존재 여부 (bHasLastDamageHitContext)", ToolTip="Dummy HitScan 또는 Projectile Actor 충돌로 마지막 Damage HitContext Debug가 기록됐는지 여부입니다."))
	bool bHasLastDamageHitContext = false;

	// [v2.97.0] 마지막 Dummy HitScan 또는 Projectile Actor 충돌 Damage HitContext입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 Damage HitContext (LastDamageHitContext)", ToolTip="마지막 Dummy HitScan 결과 또는 Projectile Actor 충돌 결과를 같은 형식으로 기록한 Debug 컨텍스트입니다. 실제 HP 차감에는 사용하지 않습니다."))
	FCFDamageHitContext LastDamageHitContext;

	// [v2.97.0] 마지막 Damage HitContext 표시 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 Damage HitContext 요약 (LastDamageHitContextSummary)", ToolTip="VehicleDebug Panel에 표시할 마지막 Damage HitContext 요약 문자열입니다."))
	FString LastDamageHitContextSummary = TEXT("DamageHitContext: None");

	// [v2.111.0] 마지막 피해 적용 결과가 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 피해 적용 결과 존재 여부 (bHasLastDamageApplyResult)", ToolTip="마지막 HitScan 또는 Projectile 명중의 피해 적용 결과가 기록됐는지 여부입니다."))
	bool bHasLastDamageApplyResult = false;

	// [v2.111.0] 마지막 직접 피해 적용과 체력 변화 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 피해 적용 결과 (LastDamageApplyResult)", ToolTip="피해 적용 여부, 거부 사유, 적용량, 체력 변화와 파괴 전환을 기록합니다."))
	FCFDamageApplyResult LastDamageApplyResult;

	// [v2.111.0] 마지막 피해 적용 결과의 한글 표시 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 피해 적용 결과 요약 (LastDamageApplyResultSummary)", ToolTip="VehicleDebug Panel에 표시할 피해 적용 결과 요약입니다."))
	FString LastDamageApplyResultSummary = TEXT("피해 적용 기록 없음");

	// [v2.130.0] 현재 Pawn이 VehicleDefenseComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="VehicleDefenseComp 보유 여부 (bHasVehicleDefenseComponent)", ToolTip="현재 Pawn이 Shield, 6방향 Armor와 Integrity 분배를 관리하는 VehicleDefenseComp를 보유하는지 표시합니다."))
	bool bHasVehicleDefenseComponent = false;

	// [v2.130.0] VehicleDefenseComp가 유효한 DefenseData로 초기화됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="VehicleDefense 초기화 여부 (bVehicleDefenseInitialized)", ToolTip="True이면 유효한 VehicleDefenseData로 Shield와 6방향 Armor가 초기화된 상태입니다."))
	bool bVehicleDefenseInitialized = false;

	// [v2.130.0] DefenseData 없이 기존 Integrity 직접 피해 호환 경로를 사용하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="Legacy 방어 Fallback 여부 (bUsingLegacyDefenseFallback)", ToolTip="True이면 VehicleDefenseData가 없어 기존 VehicleHealthComp 직접 Integrity 피해 호환 경로를 사용합니다."))
	bool bUsingLegacyDefenseFallback = false;

	// [v2.130.0] 현재 Shield, 방향별 Armor, 재생 상태와 초기화 상태 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="차량 방어 상태 요약 (VehicleDefenseSummary)", ToolTip="VehicleDefenseComp가 계산한 현재 Shield, 6방향 Armor와 재생 상태의 읽기 전용 요약입니다."))
	FString VehicleDefenseSummary = TEXT("VehicleDefenseComp 없음");

	// [v2.130.0] 현재 초기화 이후 마지막 전체 방어 피해 결과가 저장됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 차량 방어 결과 존재 여부 (bHasLastVehicleDamageResult)", ToolTip="마지막 유효 피해의 Shield, Armor, 관통과 Integrity 전체 결과가 저장됐는지 표시합니다."))
	bool bHasLastVehicleDamageResult = false;

	// [v2.130.0] 마지막 전체 방어 피해 결과의 Panel 표시용 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 차량 방어 결과 요약 (LastVehicleDamageResultSummary)", ToolTip="마지막 유효 피해의 방향, Shield 흡수, Armor 흡수·관통과 Integrity 적용 결과를 표시합니다."))
	FString LastVehicleDamageResultSummary = TEXT("차량 방어 피해 기록 없음");

	// [v2.83.0] 현재 Pawn이 ProjectilePoolComp를 보유하고 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="ProjectilePoolComp 보유 여부 (bHasProjectilePoolComponent)", ToolTip="현재 Pawn이 발사체 재사용 Pool 컴포넌트를 보유하고 있는지 여부입니다."))
	bool bHasProjectilePoolComponent = false;

	// [v2.83.0] Pool이 추적 중인 전체 Projectile Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="전체 Pool 발사체 수 (TotalPooledProjectileCount)", ToolTip="현재 Projectile Pool이 추적 중인 전체 발사체 Actor 수입니다. 활성 / 비활성 Actor를 모두 포함합니다."))
	int32 TotalPooledProjectileCount = 0;

	// [v2.83.0] Pool이 추적 중이고 현재 이동 중인 활성 Projectile Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 Pool 발사체 수 (ActivePooledProjectileCount)", ToolTip="현재 Projectile Pool이 추적 중이고 발사되어 이동 중인 발사체 Actor 수입니다."))
	int32 ActivePooledProjectileCount = 0;

	// [v2.83.0] Pool에서 재사용 대기 중인 비활성 Projectile Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="비활성 Pool 발사체 수 (InactivePooledProjectileCount)", ToolTip="현재 Projectile Pool에서 다음 발사에 재사용할 수 있는 비활성 발사체 Actor 수입니다."))
	int32 InactivePooledProjectileCount = 0;

	// [v2.85.0] Projectile Pool에 마지막으로 반환된 발사체 이벤트 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 Projectile Pool 반환 요약 (LastProjectileReleaseSummary)", ToolTip="마지막으로 Pool에 반환된 발사체의 ID, 비활성화 사유, 충돌 대상, 비행 시간을 요약한 문자열입니다."))
	FString LastProjectileReleaseSummary = TEXT("ProjectileRelease: None");

	// [v2.78.0] 현재 활성 무기에서 실제 Trace에 사용할 최대 사거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 최대 사거리 (ActiveWeaponMaxRange)", ToolTip="현재 활성 WeaponData가 유효하면 WeaponData.MaxRange, 아니면 Aim Profile MaxAimDistance입니다."))
	float ActiveWeaponMaxRange = 0.0f;

	// [v2.84.0] 현재 활성 무기의 분당 발사속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 분당 발사속도 (ActiveWeaponFireRatePerMinute)", ToolTip="현재 활성 WeaponData가 유효하면 WeaponData.FireRatePerMinute, 아니면 0입니다. 60이면 1초마다 1발입니다."))
	float ActiveWeaponFireRatePerMinute = 0.0f;

	// [v2.84.0] 현재 활성 무기의 분당 발사속도에서 환산한 발사 간격입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 발사 간격 초 (ActiveWeaponCooldownSeconds)", ToolTip="현재 활성 WeaponData의 FireRatePerMinute를 초 단위 발사 간격으로 환산한 값입니다. 기존 디버그/검증 호환용입니다."))
	float ActiveWeaponCooldownSeconds = 0.0f;

	// [v2.78.0] 현재 시간 기준 남은 무기 쿨다운 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="남은 무기 쿨다운 초 (ActiveWeaponRemainingCooldownSeconds)", ToolTip="현재 월드 시간 기준 활성 무기의 남은 쿨다운 시간입니다."))
	float ActiveWeaponRemainingCooldownSeconds = 0.0f;

	// [v2.78.0] 마지막으로 승인된 발사 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 승인 발사 시간 (LastAcceptedWeaponFireTimeSeconds)", ToolTip="VehicleWeaponComp가 마지막으로 기록한 승인 발사 시간입니다. 아직 없으면 음수입니다."))
	float LastAcceptedWeaponFireTimeSeconds = -1.0f;
};

/**
 * Pawn 레벨에서 바로 확인할 수 있는 차량 디버그 스냅샷입니다.
 * - 런타임 준비 상태와 요약 문자열
 * - 현재/이전 Drive 상태와 마지막 전이 요약
 * - DriveComp가 계산한 최신 주행 상태 스냅샷
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugSnapshot
{
	GENERATED_BODY()

	// [v2.14.1] HUD 요약용 Overview 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Overview 카테고리 (Overview)", ToolTip="HUD 요약에 사용할 VehicleDebug Overview 카테고리입니다."))
	FCFVehicleDebugOverview Overview;

	// [v2.14.1] 주행 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 카테고리 (Drive)", ToolTip="주행 상태와 입력 적용값을 담는 VehicleDebug Drive 카테고리입니다."))
	FCFVehicleDebugDrive Drive;

	// [v2.14.1] 입력 해석 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Input 카테고리 (Input)", ToolTip="입력 장치, 주도권, 2D 이동 해석 결과를 담는 VehicleDebug Input 카테고리입니다."))
	FCFVehicleDebugInput Input;

	// [v2.7.0] 카메라 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug")
	FCFVehicleDebugCamera Camera;

	// [v2.16.0] 조준 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Aim 카테고리 (Aim)", ToolTip="AimComp의 로컬/검증/표시용 조준 상태를 담는 VehicleDebug Aim 카테고리입니다."))
	FCFVehicleDebugAim Aim;

	// [v2.138.0] 선택 대상 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Target 카테고리 (Target)", ToolTip="TargetSelectComp의 현재 선택 대상 표시 정보와 대상 Actor의 방어·내구도 상태를 담는 VehicleDebug Target 카테고리입니다."))
	FCFVehicleDebugTarget Target;

	// [v2.77.0] 무기 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Weapon 카테고리 (Weapon)", ToolTip="WeaponComp의 런타임 준비 상태, WeaponData 상태, FireOrigin 결과를 담는 VehicleDebug Weapon 카테고리입니다."))
	FCFVehicleDebugWeapon Weapon;

	// [v2.14.1] 런타임 진단 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Runtime 카테고리 (Runtime)", ToolTip="런타임 준비 상태와 요약 문자열을 담는 VehicleDebug Runtime 카테고리입니다."))
	FCFVehicleDebugRuntime Runtime;

	// 기존 Pawn 런타임 초기화 완료 여부 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	// 기존 Pawn 런타임 요약 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="런타임 요약 (RuntimeSummary)", ToolTip="마지막 차량 런타임 초기화/적용 요약 문자열입니다."))
	FString RuntimeSummary = TEXT("NotInitialized");

	// Drive 컴포넌트 보유 여부 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 컴포넌트 보유 여부 (bHasDriveComponent)", ToolTip="현재 VehicleDriveComp가 유효한지 여부입니다."))
	bool bHasDriveComponent = false;

	// WheelSync 컴포넌트 보유 여부 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="WheelSync 컴포넌트 보유 여부 (bHasWheelSyncComponent)", ToolTip="현재 WheelSyncComp가 유효한지 여부입니다."))
	bool bHasWheelSyncComponent = false;

	// 현재 Drive 상태 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	// 이전 프레임 Drive 상태 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="이전 Drive 상태 (PreviousDriveState)", ToolTip="이전 프레임 기준 VehicleDriveComp의 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	// 이번 프레임 Drive 상태 변경 여부 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="이번 프레임 상태 변경 여부 (bDriveStateChangedThisFrame)", ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	// 마지막 Drive 상태 전이 요약 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="상태 전이 요약 (DriveStateTransitionSummary)", ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString DriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	// Drive 컴포넌트 최신 스냅샷 호환 필드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 상태 스냅샷 (DriveStateSnapshot)", ToolTip="VehicleDriveComp가 계산한 최신 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot DriveStateSnapshot;
};
