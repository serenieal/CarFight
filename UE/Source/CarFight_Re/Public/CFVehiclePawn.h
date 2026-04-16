// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.6.0
// Date: 2026-04-15
// Description: CarFight 신규 차량 Pawn 기준 클래스 (VehicleCameraComp / Look 입력 연동 + VehicleMove 2D 입력 해석 추가)
// Scope: DriveComp / WheelSyncComp를 소유하고 차량 DA 기준 초기 설정, 런타임 휠 튜닝, 차량 카메라 입력 전달과 차량 2D 이동 입력 해석을 함께 다룹니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleDriveComp.h"
#include "Components/SlateWrapperTypes.h"
#include "WheeledVehiclePawn.h"
#include "CFVehiclePawn.generated.h"

class UCFVehicleData;
class UCFVehicleCameraComp;
class UCFWheelSyncComp;
class UChaosWheeledVehicleMovementComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
struct FInputActionValue;
struct FKey;

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
 * 차량 2D 이동 입력 해석에 사용할 각도 설정입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMoveInputConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="쓰로틀 시작 각도 (ThrottleStartAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 쓰로틀 영역 시작 각도입니다."))
	float ThrottleStartAngleDeg = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="쓰로틀 종료 각도 (ThrottleEndAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 쓰로틀 영역 종료 각도입니다."))
	float ThrottleEndAngleDeg = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="후진 시작 각도 (ReverseStartAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 후진 영역 시작 각도입니다."))
	float ReverseStartAngleDeg = 135.0f;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="원본 이동 입력 (RawMoveInput)", ToolTip="차량 이동 Input Action에서 들어온 원본 2D 입력 벡터입니다."))
	FVector2D RawMoveInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="입력 강도 (Magnitude)", ToolTip="차량 이동 입력 벡터의 반지름 기반 강도입니다."))
	float Magnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="입력 각도 (AngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 차량 이동 입력 각도입니다."))
	float AngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="해석 영역 (ResolvedZone)", ToolTip="현재 차량 이동 입력이 해석된 영역입니다."))
	ECFVehicleMoveZone ResolvedZone = ECFVehicleMoveZone::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="해석 방향 의도 (ResolvedDirectionIntent)", ToolTip="현재 차량 이동 입력이 해석한 진행 방향 의도입니다."))
	ECFVehicleMoveDirectionIntent ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="조향 출력값 (SteeringValue)", ToolTip="현재 프레임에 DriveComp로 전달할 조향 출력값입니다."))
	float SteeringValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="스로틀 출력값 (ThrottleValue)", ToolTip="현재 프레임에 DriveComp로 전달할 스로틀 출력값입니다."))
	float ThrottleValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="브레이크 출력값 (BrakeValue)", ToolTip="현재 프레임에 DriveComp로 전달할 브레이크 출력값입니다."))
	float BrakeValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="검은 영역 유지 사용 여부 (bUsedBlackZoneHold)", ToolTip="현재 프레임에 검은 영역 방향 유지 정책이 적용되었는지 여부입니다."))
	bool bUsedBlackZoneHold = false;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="런타임 요약 (RuntimeSummary)", ToolTip="마지막 차량 런타임 초기화/적용 요약 문자열입니다."))
	FString RuntimeSummary = TEXT("NotInitialized");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 컴포넌트 보유 여부 (bHasDriveComponent)", ToolTip="현재 VehicleDriveComp가 유효한지 여부입니다."))
	bool bHasDriveComponent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="WheelSync 컴포넌트 보유 여부 (bHasWheelSyncComponent)", ToolTip="현재 WheelSyncComp가 유효한지 여부입니다."))
	bool bHasWheelSyncComponent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="이전 Drive 상태 (PreviousDriveState)", ToolTip="이전 프레임 기준 VehicleDriveComp의 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="이번 프레임 상태 변경 여부 (bDriveStateChangedThisFrame)", ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="상태 전이 요약 (DriveStateTransitionSummary)", ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString DriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 상태 스냅샷 (DriveStateSnapshot)", ToolTip="VehicleDriveComp가 계산한 최신 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot DriveStateSnapshot;
};

/**
 * CarFight 신규 차량 Pawn 기준 클래스
 * - 입력은 DriveComp로 위임합니다.
 * - 휠 시각 갱신은 WheelSyncComp로 위임합니다.
 * - 카메라 자유 조준 입력은 VehicleCameraComp로 위임합니다.
 * - BP는 얇은 조립/표현 레이어로 유지하는 것을 목표로 합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API ACFVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	// [v1.1.0] 기본 생성자
	ACFVehiclePawn();

protected:
	// [v1.4.0] Construction 시점에 VehicleVisualConfig를 적용해 에디터 미리보기를 갱신합니다.
	virtual void OnConstruction(const FTransform& Transform) override;

	// [v1.1.0] BeginPlay에서 런타임 초기화와 입력 매핑 등록을 시도합니다.
	virtual void BeginPlay() override;

	// [v1.1.0] Tick에서 휠 시각 갱신을 수행합니다.
	virtual void Tick(float DeltaSeconds) override;

	// [v1.1.0] PlayerInputComponent에 Enhanced Input Action을 바인딩합니다.
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 데이터 (VehicleData)", ToolTip="차량 루트 DataAsset 참조입니다. 현재 단계에서는 공식 타입을 UCFVehicleData로 고정하고, 세부 VehicleMovement 및 WheelVisual 해석은 후속 단계로 확장합니다."))
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="기본 입력 매핑 컨텍스트 (DefaultInputMappingContext)", ToolTip="BeginPlay와 SetupPlayerInputComponent에서 등록을 시도할 기본 Input Mapping Context 입니다."))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="차량 이동 입력 액션 (InputAction_VehicleMove)", ToolTip="차량 전진/후진/브레이크/조향을 함께 해석할 2D 이동 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_VehicleMove = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="스로틀 입력 액션 (InputAction_Throttle)", ToolTip="Throttle 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Throttle = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="조향 입력 액션 (InputAction_Steering)", ToolTip="Steering 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Steering = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="브레이크 입력 액션 (InputAction_Brake)", ToolTip="Brake 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Brake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="핸드브레이크 입력 액션 (InputAction_Handbrake)", ToolTip="Handbrake 토글 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Handbrake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="시점 입력 액션 (InputAction_Look)", ToolTip="차량 카메라 자유 조준 입력에 사용할 2D Look Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Look = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="입력 장치 모드 (InputDeviceMode)", ToolTip="차량 입력을 어떤 장치로 받을지 고정합니다. Auto는 키보드/마우스와 게임패드를 모두 허용하고, KeyboardMouseOnly는 키보드/마우스만, GamepadOnly는 게임패드만 허용합니다."))
	ECFVehicleInputDeviceMode InputDeviceMode = ECFVehicleInputDeviceMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="입력 장치 아날로그 임계값 (InputDeviceAnalogThreshold)", ToolTip="고정 입력 장치 모드에서 아날로그 키를 활성 입력으로 판정할 최소 절대값입니다. 스틱 드리프트가 있으면 값을 조금 올려서 사용합니다."))
	float InputDeviceAnalogThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(ClampMin="0", DisplayName="입력 매핑 우선순위 (InputMappingPriority)", ToolTip="기본 Input Mapping Context 등록 우선순위입니다."))
	int32 InputMappingPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="입력 매핑 자동 등록 (bAutoRegisterInputMappingContext)", ToolTip="True이면 BeginPlay / SetupPlayerInputComponent에서 기본 Input Mapping Context 등록을 자동 시도합니다."))
	bool bAutoRegisterInputMappingContext = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="차량 이동 입력 설정 (VehicleMoveInputConfig)", ToolTip="차량 2D 이동 입력의 각도 해석 기본 설정입니다."))
	FCFVehicleMoveInputConfig VehicleMoveInputConfig;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 진행 방향 의도 (LastMoveDirectionIntent)", ToolTip="검은 영역 진입 시 유지할 마지막 유효 진행 방향입니다."))
	ECFVehicleMoveDirectionIntent LastMoveDirectionIntent = ECFVehicleMoveDirectionIntent::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 차량 이동 입력 결과 (LastVehicleMoveInputResult)", ToolTip="현재 프레임 기준 차량 2D 이동 입력 해석 결과입니다."))
	FCFVehicleMoveInputResult LastVehicleMoveInputResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="Drive 컴포넌트 (VehicleDriveComp)", ToolTip="입력 전달 전용 Drive 컴포넌트입니다."))
	TObjectPtr<UCFVehicleDriveComp> VehicleDriveComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="WheelSync 컴포넌트 (WheelSyncComp)", ToolTip="휠 시각 동기화 전용 WheelSync 컴포넌트입니다."))
	TObjectPtr<UCFWheelSyncComp> WheelSyncComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="VehicleCamera 컴포넌트 (VehicleCameraComp)", ToolTip="차량 중심 피벗 기반 자유 조준과 Aim Trace를 계산하는 차량 카메라 컴포넌트입니다."))
	TObjectPtr<UCFVehicleCameraComp> VehicleCameraComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="BeginPlay 자동 초기화 (bAutoInitializeOnBeginPlay)", ToolTip="True이면 BeginPlay에서 Drive / WheelSync 캐시와 준비를 자동 시도합니다."))
	bool bAutoInitializeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="휠 시각 Tick 사용 (bEnableWheelVisualTick)", ToolTip="True이면 Tick에서 WheelSync의 휠 시각 갱신을 자동 호출합니다."))
	bool bEnableWheelVisualTick = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="차량 런타임 준비 완료 여부 (bVehicleRuntimeReady)", ToolTip="Drive / WheelSync 준비가 성공했는지 여부입니다."))
	bool bVehicleRuntimeReady = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="런타임 결과 요약 (LastVehicleRuntimeSummary)", ToolTip="마지막 런타임 초기화 결과 요약 문자열입니다."))
	FString LastVehicleRuntimeSummary = TEXT("NotInitialized");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="화면 DriveState 디버그 사용 (bEnableDriveStateOnScreenDebug)", ToolTip="True이면 PIE 중 현재 Drive 상태와 속도를 화면에 표시합니다."))
	bool bEnableDriveStateOnScreenDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="DriveState 디버그 표시 모드 (DriveStateDebugDisplayMode)", ToolTip="PIE 중 화면에 표시할 Drive 상태 디버그 문자열 포맷을 선택합니다. Off는 비표시, SingleLine은 한 줄 요약, MultiLine은 줄바꿈 상세 표시입니다."))
	ECFVehicleDebugDisplayMode DriveStateDebugDisplayMode = ECFVehicleDebugDisplayMode::SingleLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="상태 전이 요약 표시 (bShowDriveStateTransitionSummary)", ToolTip="True이면 현재 Drive 상태와 함께 마지막 상태 전이 요약 문자열도 화면에 표시합니다."))
	bool bShowDriveStateTransitionSummary = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(ClampMin="0.0", DisplayName="디버그 메시지 유지 시간 (DriveStateDebugMessageDuration)", ToolTip="화면에 표시하는 Drive 상태 디버그 메시지 유지 시간(초)입니다."))
	float DriveStateDebugMessageDuration = 0.0f;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 입력 전달 전용 DriveComp를 반환합니다."))
	UCFVehicleDriveComp* GetVehicleDriveComp() const { return VehicleDriveComp; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 휠 시각 동기화 전용 WheelSyncComp를 반환합니다."))
	UCFWheelSyncComp* GetWheelSyncComp() const { return WheelSyncComp; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 중심 피벗 기반 자유 조준을 계산하는 VehicleCameraComp를 반환합니다."))
	UCFVehicleCameraComp* GetVehicleCameraComp() const { return VehicleCameraComp; }

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="현재 플레이어 컨트롤러 기준으로 기본 Input Mapping Context 등록을 시도합니다."))
	bool RegisterDefaultInputMappingContext();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn", meta=(ToolTip="Drive / WheelSync 캐시와 준비를 다시 시도합니다."))
	bool InitializeVehicleRuntime();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn", meta=(ToolTip="WheelSync의 캐시와 준비를 다시 시도합니다."))
	bool PrepareWheelSync();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn", meta=(ToolTip="WheelSync를 사용해 휠 시각 갱신을 한 번 실행합니다."))
	bool UpdateVehicleWheelVisuals(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Throttle 입력을 DriveComp로 전달합니다."))
	void SetVehicleThrottleInput(float InThrottleValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Steering 입력을 DriveComp로 전달합니다."))
	void SetVehicleSteeringInput(float InSteeringValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Brake 입력을 DriveComp로 전달합니다."))
	void SetVehicleBrakeInput(float InBrakeValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Handbrake 입력을 DriveComp로 전달합니다."))
	void SetVehicleHandbrakeInput(bool bInHandbrakePressed);

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 전체 속도(km/h)를 반환합니다."))
	float GetVehicleSpeed() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 Drive 상태를 반환합니다."))
	ECFVehicleDriveState GetDriveState() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 Drive 상태 스냅샷을 반환합니다."))
	FCFVehicleDriveStateSnapshot GetDriveStateSnapshot() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="Pawn 기준 런타임 준비 상태, Drive 상태, 마지막 전이 요약을 하나로 묶은 디버그 스냅샷을 반환합니다."))
	FCFVehicleDebugSnapshot GetVehicleDebugSnapshot() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="Debug Widget의 단일 라인 Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다."))
	FText GetDebugTextSingleLine() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="Debug Widget의 멀티라인 Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다."))
	FText GetDebugTextMultiLine() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 DriveStateDebugDisplayMode 설정을 따라 Debug Widget Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다. Off이면 빈 텍스트를 반환합니다."))
	FText GetDebugTextByDisplayMode() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 Debug Widget을 표시해야 하는지 여부를 반환합니다. OnScreenDebug가 꺼져 있거나 DisplayMode가 Off이면 False를 반환합니다."))
	bool ShouldShowDebugWidget() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 Debug Widget Visibility 바인딩에 바로 사용할 수 있는 값을 반환합니다. 표시 조건을 만족하면 HitTestInvisible, 아니면 Collapsed를 반환합니다."))
	ESlateVisibility GetDebugWidgetVisibility() const;

protected:
	// [v1.5.0] DriveComp 캐시를 재사용해 VehicleMovement 컴포넌트를 안전하게 가져옵니다.
	UChaosWheeledVehicleMovementComponent* ResolveVehicleMovementComponent(const TCHAR* CacheFailureSummary, const TCHAR* MissingComponentSummary);

	// [v1.6.0] 현재 설정에 맞는 차량 디버그 요약 문자열을 생성합니다.
	FString BuildVehicleDebugSummary(bool bUseMultilineFormat, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const;

	// [v1.5.0] WheelSync 실행 결과를 기존 런타임 요약 뒤에 덧붙입니다.
	void AppendWheelSyncRuntimeSummary();

	// [v1.5.0] 축 입력 액션값을 읽어 허용 여부를 검사한 뒤 지정 setter로 전달합니다.
	void ApplyAxisInputFromAction(const UInputAction* SourceInputAction, const FInputActionValue& InputActionValue, void (ACFVehiclePawn::*AxisInputSetter)(float));

	// [v1.5.0] 지정된 축 입력 setter를 0으로 초기화합니다.
	void ResetAxisInput(void (ACFVehiclePawn::*AxisInputSetter)(float));

	// [v1.6.0] 2D 이동 입력 벡터를 위=0도, 시계 방향 증가 각도로 변환합니다.
	float ConvertMoveInputToAngleDeg(const FVector2D& MoveInputVector) const;

	// [v1.6.0] 주어진 각도가 시작/종료 각도 포함 범위 안에 들어오는지 검사합니다.
	bool IsAngleWithinRange(float InAngleDeg, float StartAngleDeg, float EndAngleDeg) const;

	// [v1.6.0] 현재 속도 기준으로 마지막 진행 방향 의도의 fallback 값을 계산합니다.
	ECFVehicleMoveDirectionIntent ResolveDirectionIntentFallback() const;

	// [v1.6.0] 2D 이동 입력 벡터를 차량 이동 해석 결과로 변환합니다.
	FCFVehicleMoveInputResult ResolveVehicleMoveInput(const FVector2D& MoveInputVector) const;

	// [v1.6.0] 해석된 차량 이동 입력 결과를 DriveComp 입력으로 적용합니다.
	void ApplyResolvedVehicleMoveInput(const FCFVehicleMoveInputResult& ResolvedMoveInput);

	void ApplyVehicleDataConfig();
	void ApplyVehicleVisualConfig();
	void ApplyVehicleLayoutConfig();
	void ApplyVehicleMovementConfig();
	// [v1.2.0] VehicleMovementConfig 중 런타임 setter가 가능한 휠 물리 값을 차량 인스턴스 기준으로 적용합니다.
	void ApplyVehicleWheelPhysicsConfig();
	void ApplyVehicleWheelVisualConfig();
	void ApplyVehicleReferenceConfig();
	void DisplayDriveStateOnScreenDebug() const;
	bool ShouldAcceptActionInput(const UInputAction* SourceInputAction, float CurrentInputValue) const;
	bool HasActiveMappedKeyForDevice(const UInputAction* SourceInputAction, bool bRequireGamepadKey) const;
	bool IsMappedKeyCurrentlyActive(const FKey& MappingKey) const;

	// [v1.6.0] 차량 이동용 2D 입력 액션값을 읽어 해석 결과를 Drive 입력으로 전달합니다.
	void HandleVehicleMoveInput(const FInputActionValue& InputActionValue);

	// [v1.6.0] 차량 이동용 2D 입력 액션이 해제됐을 때 이동 입력 상태를 초기화합니다.
	void HandleVehicleMoveReleased(const FInputActionValue& InputActionValue);

	void HandleThrottleInput(const FInputActionValue& InputActionValue);
	void HandleThrottleReleased(const FInputActionValue& InputActionValue);
	void HandleSteeringInput(const FInputActionValue& InputActionValue);
	void HandleSteeringReleased(const FInputActionValue& InputActionValue);
	void HandleBrakeInput(const FInputActionValue& InputActionValue);
	void HandleBrakeReleased(const FInputActionValue& InputActionValue);
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleLookReleased(const FInputActionValue& InputActionValue);
	void HandleHandbrakeStarted(const FInputActionValue& InputActionValue);
	void HandleHandbrakeCompleted(const FInputActionValue& InputActionValue);
};
