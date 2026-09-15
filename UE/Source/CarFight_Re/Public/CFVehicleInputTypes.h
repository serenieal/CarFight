// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-14
// Description: CarFight 차량 입력 reflected type declaration owner
// Changelog:
// - v1.0.0: CF-FQ-048 VPS-P0-04에서 CFVehiclePawn.h가 소유하던 차량 입력 enum/struct 선언을 독립 Public type header로 이동.
// Migration:
// - 기존 reflected 이름, enum 값, Blueprint 노출, UPROPERTY 이름/타입/메타데이터와 기본값 의미는 변경하지 않습니다. 기존 C++/Blueprint 호출부 수정은 필요하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleInputTypes.generated.h"

/**
 * 차량 입력에 사용할 장치 모드입니다.
 * - Auto: 키보드/마우스와 게임패드를 모두 허용합니다.
 * - KeyboardMouseOnly: 키보드/마우스 입력만 허용합니다.
 * - GamepadOnly: 게임패드 입력만 허용합니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleInputDeviceMode : uint8
{
	Auto UMETA(DisplayName="Auto"),
	KeyboardMouseOnly UMETA(DisplayName="KeyboardMouseOnly"),
	GamepadOnly UMETA(DisplayName="GamepadOnly")
};

/**
 * 차량 2D 이동 입력에서 마지막으로 유효했던 진행 방향 의도입니다.
 * - None: 아직 유효한 진행 방향 의도가 없습니다.
 * - Forward: 최근 유효 입력이 전진 의도였습니다.
 * - Reverse: 최근 유효 입력이 후진 의도였습니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleMoveDirectionIntent : uint8
{
	None UMETA(DisplayName="None"),
	Forward UMETA(DisplayName="Forward"),
	Reverse UMETA(DisplayName="Reverse")
};

/**
 * 차량 2D 이동 입력이 해석된 영역입니다.
 * - None: 아직 어떤 영역으로도 해석되지 않았습니다.
 * - Throttle: 전진 쓰로틀 영역입니다.
 * - Reverse: 후진/브레이크 해석 영역입니다.
 * - Black: 방향 유지 완충용 검은 영역입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleMoveZone : uint8
{
	None UMETA(DisplayName="None"),
	Throttle UMETA(DisplayName="Throttle"),
	Reverse UMETA(DisplayName="Reverse"),
	Black UMETA(DisplayName="Black")
};

/**
 * 현재 프레임 기준 차량 입력 적용의 주도권을 가진 경로입니다.
 * - None: 아직 어떤 입력 경로도 주도권을 가지지 않습니다.
 * - VehicleMove2D: `IA_VehicleMove` 기반 2D 신규 조작이 주도권을 가집니다.
 * - LegacyAxis: 기존 `Throttle / Brake / Steering` 축 입력이 주도권을 가집니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleInputOwnership : uint8
{
	None UMETA(DisplayName="None"),
	VehicleMove2D UMETA(DisplayName="VehicleMove2D"),
	LegacyAxis UMETA(DisplayName="LegacyAxis")
};

/**
 * 차량 2D 이동 입력 해석에 사용할 각도 설정입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMoveInputConfig
{
	GENERATED_BODY()

	// 쓰로틀 영역 시작 각도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="쓰로틀 시작 각도 (ThrottleStartAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 쓰로틀 영역 시작 각도입니다."))
	float ThrottleStartAngleDeg = 260.0f;

	// 쓰로틀 영역 종료 각도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="쓰로틀 종료 각도 (ThrottleEndAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 쓰로틀 영역 종료 각도입니다."))
	float ThrottleEndAngleDeg = 100.0f;

	// 후진 영역 시작 각도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="후진 시작 각도 (ReverseStartAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 후진 영역 시작 각도입니다."))
	float ReverseStartAngleDeg = 135.0f;

	// 후진 영역 종료 각도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="후진 종료 각도 (ReverseEndAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 후진 영역 종료 각도입니다."))
	float ReverseEndAngleDeg = 225.0f;
};

/**
 * 차량 2D 이동 입력의 최신 해석 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMoveInputResult
{
	GENERATED_BODY()

	// 차량 이동 Input Action에서 들어온 원본 2D 입력 벡터입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="원본 이동 입력 (RawMoveInput)", ToolTip="차량 이동 Input Action에서 들어온 원본 2D 입력 벡터입니다."))
	FVector2D RawMoveInput = FVector2D::ZeroVector;

	// 차량 이동 입력 벡터의 반지름 기반 강도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="입력 강도 (Magnitude)", ToolTip="차량 이동 입력 벡터의 반지름 기반 강도입니다."))
	float Magnitude = 0.0f;

	// 위가 0도, 시계 방향 증가 기준의 입력 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="입력 각도 (AngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 차량 이동 입력 각도입니다."))
	float AngleDeg = 0.0f;

	// 현재 차량 이동 입력이 해석된 영역입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="해석 영역 (ResolvedZone)", ToolTip="현재 차량 이동 입력이 해석된 영역입니다."))
	ECFVehicleMoveZone ResolvedZone = ECFVehicleMoveZone::None;

	// 현재 차량 이동 입력이 해석한 진행 방향 의도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="해석 방향 의도 (ResolvedDirectionIntent)", ToolTip="현재 차량 이동 입력이 해석한 진행 방향 의도입니다."))
	ECFVehicleMoveDirectionIntent ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::None;

	// 현재 프레임에 DriveComp로 전달할 조향 출력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="조향 출력값 (SteeringValue)", ToolTip="현재 프레임에 DriveComp로 전달할 조향 출력값입니다."))
	float SteeringValue = 0.0f;

	// 현재 프레임에 DriveComp로 전달할 스로틀 출력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="스로틀 출력값 (ThrottleValue)", ToolTip="현재 프레임에 DriveComp로 전달할 스로틀 출력값입니다."))
	float ThrottleValue = 0.0f;

	// 현재 프레임에 DriveComp로 전달할 브레이크 출력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="브레이크 출력값 (BrakeValue)", ToolTip="현재 프레임에 DriveComp로 전달할 브레이크 출력값입니다."))
	float BrakeValue = 0.0f;

	// 현재 프레임에 검은 영역 방향 유지 정책이 적용되었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="검은 영역 유지 사용 여부 (bUsedBlackZoneHold)", ToolTip="현재 프레임에 검은 영역 방향 유지 정책이 적용되었는지 여부입니다."))
	bool bUsedBlackZoneHold = false;
};
