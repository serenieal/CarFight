// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.3.0
// Date: 2026-03-19
// Description: CarFight 신규 차량 Pawn 기준 클래스
// Scope: DriveComp / WheelSyncComp를 소유하고 새 BP의 얇은 부모로 동작합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleDriveComp.h"
#include "Components/SlateWrapperTypes.h"
#include "WheeledVehiclePawn.h"
#include "CFVehiclePawn.generated.h"

class UCFVehicleData;
class UCFWheelSyncComp;
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
 * Pawn 레벨에서 바로 확인할 수 있는 차량 디버그 스냅샷입니다.
 * - 런타임 준비 상태와 요약 문자열
 * - 현재/이전 Drive 상태와 마지막 전이 요약
 * - DriveComp가 계산한 최신 주행 상태 스냅샷
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="마지막 차량 런타임 초기화/적용 요약 문자열입니다."))
	FString RuntimeSummary = TEXT("NotInitialized");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 VehicleDriveComp가 유효한지 여부입니다."))
	bool bHasDriveComponent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 WheelSyncComp가 유효한지 여부입니다."))
	bool bHasWheelSyncComponent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="이전 프레임 기준 VehicleDriveComp의 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString DriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle|Debug", meta=(ToolTip="VehicleDriveComp가 계산한 최신 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot DriveStateSnapshot;
};

/**
 * CarFight 신규 차량 Pawn 기준 클래스
 * - 입력은 DriveComp로 위임합니다.
 * - 휠 시각 갱신은 WheelSyncComp로 위임합니다.
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle", meta=(ToolTip="차량 루트 DataAsset 참조입니다. 현재 단계에서는 공식 타입을 UCFVehicleData로 고정하고, 세부 VehicleMovement 및 WheelVisual 해석은 후속 단계로 확장합니다."))
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Input", meta=(ToolTip="BeginPlay와 SetupPlayerInputComponent에서 등록을 시도할 기본 Input Mapping Context 입니다."))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Input", meta=(ToolTip="Throttle 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Throttle = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Input", meta=(ToolTip="Steering 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Steering = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Input", meta=(ToolTip="Brake 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Brake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Input", meta=(ToolTip="Handbrake 토글 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Handbrake = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Input", meta=(ToolTip="차량 입력을 어떤 장치로 받을지 고정합니다. Auto는 키보드/마우스와 게임패드를 모두 허용하고, KeyboardMouseOnly는 키보드/마우스만, GamepadOnly는 게임패드만 허용합니다."))
	ECFVehicleInputDeviceMode InputDeviceMode = ECFVehicleInputDeviceMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Input", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="고정 입력 장치 모드에서 아날로그 키를 활성 입력으로 판정할 최소 절대값입니다. 스틱 드리프트가 있으면 값을 조금 올려서 사용합니다."))
	float InputDeviceAnalogThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Input", meta=(ClampMin="0", ToolTip="기본 Input Mapping Context 등록 우선순위입니다."))
	int32 InputMappingPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Input", meta=(ToolTip="True이면 BeginPlay / SetupPlayerInputComponent에서 기본 Input Mapping Context 등록을 자동 시도합니다."))
	bool bAutoRegisterInputMappingContext = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle", meta=(AllowPrivateAccess="true", ToolTip="입력 전달 전용 Drive 컴포넌트입니다."))
	TObjectPtr<UCFVehicleDriveComp> VehicleDriveComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle", meta=(AllowPrivateAccess="true", ToolTip="휠 시각 동기화 전용 WheelSync 컴포넌트입니다."))
	TObjectPtr<UCFWheelSyncComp> WheelSyncComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Runtime", meta=(ToolTip="True이면 BeginPlay에서 Drive / WheelSync 캐시와 준비를 자동 시도합니다."))
	bool bAutoInitializeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Runtime", meta=(ToolTip="True이면 Tick에서 WheelSync의 휠 시각 갱신을 자동 호출합니다."))
	bool bEnableWheelVisualTick = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Runtime", meta=(ToolTip="Drive / WheelSync 준비가 성공했는지 여부입니다."))
	bool bVehicleRuntimeReady = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Vehicle|Runtime", meta=(ToolTip="마지막 런타임 초기화 결과 요약 문자열입니다."))
	FString LastVehicleRuntimeSummary = TEXT("NotInitialized");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Debug", meta=(ToolTip="True이면 PIE 중 현재 Drive 상태와 속도를 화면에 표시합니다."))
	bool bEnableDriveStateOnScreenDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Debug", meta=(ToolTip="PIE 중 화면에 표시할 Drive 상태 디버그 문자열 포맷을 선택합니다. Off는 비표시, SingleLine은 한 줄 요약, MultiLine은 줄바꿈 상세 표시입니다."))
	ECFVehicleDebugDisplayMode DriveStateDebugDisplayMode = ECFVehicleDebugDisplayMode::SingleLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Debug", meta=(ToolTip="True이면 현재 Drive 상태와 함께 마지막 상태 전이 요약 문자열도 화면에 표시합니다."))
	bool bShowDriveStateTransitionSummary = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle|Debug", meta=(ClampMin="0.0", ToolTip="화면에 표시하는 Drive 상태 디버그 메시지 유지 시간(초)입니다."))
	float DriveStateDebugMessageDuration = 0.0f;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle", meta=(ToolTip="차량 입력 전달 전용 DriveComp를 반환합니다."))
	UCFVehicleDriveComp* GetVehicleDriveComp() const { return VehicleDriveComp; }

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle", meta=(ToolTip="차량 휠 시각 동기화 전용 WheelSyncComp를 반환합니다."))
	UCFWheelSyncComp* GetWheelSyncComp() const { return WheelSyncComp; }

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle|Input", meta=(ToolTip="현재 플레이어 컨트롤러 기준으로 기본 Input Mapping Context 등록을 시도합니다."))
	bool RegisterDefaultInputMappingContext();

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle", meta=(ToolTip="Drive / WheelSync 캐시와 준비를 다시 시도합니다."))
	bool InitializeVehicleRuntime();

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle", meta=(ToolTip="WheelSync의 캐시와 준비를 다시 시도합니다."))
	bool PrepareWheelSync();

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle", meta=(ToolTip="WheelSync를 사용해 휠 시각 갱신을 한 번 실행합니다."))
	bool UpdateVehicleWheelVisuals(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle|Input", meta=(ToolTip="Throttle 입력을 DriveComp로 전달합니다."))
	void SetVehicleThrottleInput(float InThrottleValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle|Input", meta=(ToolTip="Steering 입력을 DriveComp로 전달합니다."))
	void SetVehicleSteeringInput(float InSteeringValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle|Input", meta=(ToolTip="Brake 입력을 DriveComp로 전달합니다."))
	void SetVehicleBrakeInput(float InBrakeValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle|Input", meta=(ToolTip="Handbrake 입력을 DriveComp로 전달합니다."))
	void SetVehicleHandbrakeInput(bool bInHandbrakePressed);

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 전체 속도(km/h)를 반환합니다."))
	float GetVehicleSpeed() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 Drive 상태를 반환합니다."))
	ECFVehicleDriveState GetDriveState() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 Drive 상태 스냅샷을 반환합니다."))
	FCFVehicleDriveStateSnapshot GetDriveStateSnapshot() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Pawn 기준 런타임 준비 상태, Drive 상태, 마지막 전이 요약을 하나로 묶은 디버그 스냅샷을 반환합니다."))
	FCFVehicleDebugSnapshot GetVehicleDebugSnapshot() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Debug Widget의 단일 라인 Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다."))
	FText GetDebugTextSingleLine() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Debug Widget의 멀티라인 Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다."))
	FText GetDebugTextMultiLine() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 DriveStateDebugDisplayMode 설정을 따라 Debug Widget Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다. Off이면 빈 텍스트를 반환합니다."))
	FText GetDebugTextByDisplayMode() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 Debug Widget을 표시해야 하는지 여부를 반환합니다. OnScreenDebug가 꺼져 있거나 DisplayMode가 Off이면 False를 반환합니다."))
	bool ShouldShowDebugWidget() const;

	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="현재 Debug Widget Visibility 바인딩에 바로 사용할 수 있는 값을 반환합니다. 표시 조건을 만족하면 HitTestInvisible, 아니면 Collapsed를 반환합니다."))
	ESlateVisibility GetDebugWidgetVisibility() const;

protected:
	void ApplyVehicleDataConfig();
	void ApplyVehicleVisualConfig();
	void ApplyVehicleLayoutConfig();
	void ApplyVehicleMovementConfig();
	void ApplyVehicleWheelVisualConfig();
	void ApplyVehicleReferenceConfig();
	void DisplayDriveStateOnScreenDebug() const;
	bool ShouldAcceptActionInput(const UInputAction* SourceInputAction, float CurrentInputValue) const;
	bool HasActiveMappedKeyForDevice(const UInputAction* SourceInputAction, bool bRequireGamepadKey) const;
	bool IsMappedKeyCurrentlyActive(const FKey& MappingKey) const;
	void HandleThrottleInput(const FInputActionValue& InputActionValue);
	void HandleThrottleReleased(const FInputActionValue& InputActionValue);
	void HandleSteeringInput(const FInputActionValue& InputActionValue);
	void HandleSteeringReleased(const FInputActionValue& InputActionValue);
	void HandleBrakeInput(const FInputActionValue& InputActionValue);
	void HandleBrakeReleased(const FInputActionValue& InputActionValue);
	void HandleHandbrakeStarted(const FInputActionValue& InputActionValue);
	void HandleHandbrakeCompleted(const FInputActionValue& InputActionValue);
};
