// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.1.0
// Date: 2026-04-10
// Description: CarFight 차량 카메라 컴포넌트 초안
// Scope: 차량 중심 피벗 기반 자유 조준, 제한각 Clamp, SpringArm 연동, Aim Trace 계산 골격을 제공합니다.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFVehicleCameraData.h"
#include "CFVehicleCameraComp.generated.h"

class ACFVehiclePawn;
class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

/**
 * CarFight 차량 카메라 시스템의 핵심 계산 컴포넌트입니다.
 * - 차량 이동과 카메라 회전을 분리합니다.
 * - 차량 중심 피벗 기준 자유 조준을 계산합니다.
 * - 현재 Aim Profile 기준으로 Yaw/Pitch를 Clamp 합니다.
 * - SpringArm / FollowCamera에 최종 Transform과 FOV를 반영합니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleCameraComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v0.1.0] 기본 생성자
	UCFVehicleCameraComp();

protected:
	// [v0.1.0] BeginPlay에서 카메라 런타임 초기화를 시도합니다.
	virtual void BeginPlay() override;

public:
	// [v0.1.0] Tick에서 차량 상태를 읽어 카메라를 갱신합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 차량 공통 카메라 기본값을 제공하는 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera", meta=(DisplayName="카메라 데이터 (VehicleCameraData)", ToolTip="차량 공통 카메라 튜닝값과 기본 Aim Profile을 제공할 DataAsset 입니다."))
	TObjectPtr<UCFVehicleCameraData> VehicleCameraData = nullptr;

	// 차량 중심 피벗 기준점 Scene Component 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|References", meta=(DisplayName="카메라 Pivot Root (CameraPivotRoot)", ToolTip="차량 중심 수평 피벗 기준점으로 사용할 Scene Component 입니다. 비어 있으면 Owner Actor 위치를 기준으로 사용합니다."))
	TObjectPtr<USceneComponent> CameraPivotRoot = nullptr;

	// 실제 자유 조준 회전이 적용되는 Aim Pivot Scene Component 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|References", meta=(DisplayName="카메라 Aim Pivot (CameraAimPivot)", ToolTip="실제 자유 조준 회전을 적용할 Scene Component 입니다. 비어 있으면 계산만 수행합니다."))
	TObjectPtr<USceneComponent> CameraAimPivot = nullptr;

	// 외부 3인칭 위치와 충돌 처리를 담당할 Spring Arm 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|References", meta=(DisplayName="카메라 Boom (CameraBoom)", ToolTip="외부 3인칭 거리와 충돌 테스트를 담당할 Spring Arm 입니다."))
	TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

	// 최종 시야를 제공할 Follow Camera 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|References", meta=(DisplayName="팔로우 카메라 (FollowCamera)", ToolTip="최종 FOV와 월드 시야를 제공할 Camera Component 입니다."))
	TObjectPtr<UCameraComponent> FollowCamera = nullptr;

	// BeginPlay에서 자동으로 참조 검색과 기본 상태 설정을 시도할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="BeginPlay 자동 초기화 (bAutoInitializeOnBeginPlay)", ToolTip="True이면 BeginPlay에서 카메라 참조 검색과 기본 상태 설정을 자동 시도합니다."))
	bool bAutoInitializeOnBeginPlay = true;

	// CameraPivotRoot를 자동으로 찾을 때 사용할 기본 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Pivot Root 기본 이름 (DefaultPivotRootName)", ToolTip="CameraPivotRoot가 비어 있을 때 Owner에서 자동 검색할 기본 Scene Component 이름입니다."))
	FName DefaultPivotRootName = TEXT("CameraPivotRoot");

	// CameraAimPivot를 자동으로 찾을 때 사용할 기본 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="Aim Pivot 기본 이름 (DefaultAimPivotName)", ToolTip="CameraAimPivot가 비어 있을 때 Owner에서 자동 검색할 기본 Scene Component 이름입니다."))
	FName DefaultAimPivotName = TEXT("CameraAimPivot");

	// CameraBoom을 자동으로 찾을 때 사용할 기본 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="CameraBoom 기본 이름 (DefaultCameraBoomName)", ToolTip="CameraBoom이 비어 있을 때 Owner에서 자동 검색할 기본 Spring Arm 이름입니다."))
	FName DefaultCameraBoomName = TEXT("CameraBoom");

	// FollowCamera를 자동으로 찾을 때 사용할 기본 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="FollowCamera 기본 이름 (DefaultFollowCameraName)", ToolTip="FollowCamera가 비어 있을 때 Owner에서 자동 검색할 기본 Camera Component 이름입니다."))
	FName DefaultFollowCameraName = TEXT("FollowCamera");

	// 현재 카메라 런타임 디버그 스냅샷입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="카메라 런타임 상태 (CameraRuntimeState)", ToolTip="현재 카메라 모드, 조준 각도, Arm 길이, FOV, Aim Trace 상태를 묶은 런타임 스냅샷입니다."))
	FCFVehicleCameraRuntimeState CameraRuntimeState;

	// 현재 카메라 모드 Modifier 플래그입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Runtime", meta=(DisplayName="카메라 모드 플래그 (CameraModeFlags)", ToolTip="전투, 후진, 공중, 파괴, 관전자 상태를 외부 시스템이 전달할 수 있는 모드 플래그입니다."))
	FCFVehicleCameraModeFlags CameraModeFlags;

	// 디버그용 Aim Trace 선을 그릴지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Vehicle Camera|Debug", meta=(DisplayName="Aim Trace 디버그 사용 (bDrawAimTraceDebug)", ToolTip="True이면 Aim Trace를 월드 디버그 라인으로 표시합니다."))
	bool bDrawAimTraceDebug = false;

public:
	// [v0.1.0] 카메라 참조 검색과 기본 상태 설정을 다시 시도합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera", meta=(ToolTip="카메라 참조 검색과 기본 런타임 상태 초기화를 다시 시도합니다."))
	bool InitializeCameraRuntime();

	// [v0.1.0] 현재 프레임 Look 입력값을 교체합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Input", meta=(ToolTip="현재 프레임에 사용할 Look 입력값을 교체합니다. 일반적으로 Enhanced Input Triggered 이벤트에서 호출합니다."))
	void SetLookInput(FVector2D InLookInput);

	// [v0.1.0] 현재 프레임 Look 입력값에 추가 입력을 누적합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Input", meta=(ToolTip="현재 프레임에 사용할 Look 입력값에 추가 입력을 누적합니다."))
	void AddLookInput(float InLookInputX, float InLookInputY);

	// [v0.1.0] 현재 프레임 Look 입력을 0으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Input", meta=(ToolTip="현재 프레임에 사용할 Look 입력값을 0으로 초기화합니다. 일반적으로 Enhanced Input Completed 이벤트에서 호출합니다."))
	void ClearLookInput();

	// [v0.1.0] 현재 조준을 차량 정면 기준으로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Input", meta=(ToolTip="현재 누적 Aim Yaw/Pitch를 0으로 초기화해 차량 정면 기준으로 조준을 되돌립니다."))
	void ResetAimToVehicleForward();

	// [v0.1.0] 외부 시스템이 카메라 모드 Modifier 플래그를 한 번에 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Mode", meta=(ToolTip="외부 시스템이 카메라 모드 Modifier 플래그를 한 번에 갱신합니다."))
	void SetCameraModeFlags(const FCFVehicleCameraModeFlags& InCameraModeFlags);

	// [v0.1.0] 외부 시스템이 임시 Aim Profile을 주입합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Aim", meta=(ToolTip="외부 무기 시스템이 현재 활성 무기 그룹의 Aim Profile을 임시로 주입합니다."))
	void SetAimProfileOverride(const FCFVehicleCameraAimProfile& InAimProfile);

	// [v0.1.0] 임시 Aim Profile 주입 상태를 해제합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle Camera|Aim", meta=(ToolTip="외부 무기 시스템이 주입한 임시 Aim Profile을 해제하고 DataAsset 기본값으로 되돌립니다."))
	void ClearAimProfileOverride();

	// [v0.1.0] 현재 카메라 런타임 스냅샷을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle Camera", meta=(ToolTip="현재 카메라 모드, 조준 각도, Arm 길이, FOV, Aim Trace 상태를 묶은 런타임 스냅샷을 반환합니다."))
	FCFVehicleCameraRuntimeState GetCameraRuntimeState() const;

	// [v0.1.0] 현재 해석된 Aim Profile을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle Camera|Aim", meta=(ToolTip="현재 적용 중인 Aim Profile을 반환합니다. 임시 Override가 있으면 이를 우선 사용합니다."))
	FCFVehicleCameraAimProfile GetResolvedAimProfile() const;

	// [v0.1.0] 현재 Clamp 적용 후 Aim Rotation을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle Camera|Aim", meta=(ToolTip="현재 Clamp 적용 후 차량 기준 Aim Rotation을 반환합니다."))
	FRotator GetCurrentAimRotation() const;

	// [v0.1.0] 현재 카메라가 향하는 월드 방향 벡터를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle Camera|Aim", meta=(ToolTip="현재 카메라가 향하는 월드 방향 벡터를 반환합니다."))
	FVector GetCurrentAimDirection() const;

	// [v0.1.0] 현재 Aim Trace 적중 위치를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle Camera|Aim", meta=(ToolTip="현재 Aim Trace가 계산한 월드 적중 위치를 반환합니다."))
	FVector GetCurrentAimHitLocation() const;

protected:
	// [v0.1.0] Owner가 ACFVehiclePawn인지 확인하고 캐시합니다.
	ACFVehiclePawn* ResolveVehiclePawnOwner();

	// [v0.1.0] Owner에서 필요한 카메라 컴포넌트 참조를 자동으로 검색합니다.
	bool ResolveCameraReferences();

	// [v0.1.0] 현재 카메라 모드를 평가합니다.
	ECFVehicleCameraMode EvaluateCameraMode() const;

	// [v0.1.0] 현재 사용할 Aim Profile을 조합해 반환합니다.
	FCFVehicleCameraAimProfile BuildResolvedAimProfile() const;

	// [v0.1.0] 현재 차량 속도(km/h)를 읽어옵니다.
	float GetVehicleSpeedKmh() const;

	// [v0.1.0] 차량 기준 기본 Aim Rotation을 계산합니다.
	FRotator GetBaseVehicleAimRotation() const;

	// [v0.1.0] 카메라 피벗의 월드 위치를 계산합니다.
	FVector GetPivotWorldLocation(const FCFVehicleCameraTuningConfig& CameraTuningConfig) const;

	// [v0.1.0] 입력을 누적하고 Aim Yaw/Pitch를 Clamp 합니다.
	void UpdateAimState(float DeltaTime, const FCFVehicleCameraTuningConfig& CameraTuningConfig, const FCFVehicleCameraAimProfile& AimProfile);

	// [v0.1.0] 목표 카메라 위치와 FOV를 계산해 SpringArm / Camera에 반영합니다.
	void UpdateCameraTransform(float DeltaTime, const FCFVehicleCameraTuningConfig& CameraTuningConfig, const FCFVehicleCameraAimProfile& AimProfile);

	// [v0.1.0] 현재 카메라 기준 Aim Trace를 계산합니다.
	void UpdateAimTrace(const FCFVehicleCameraTuningConfig& CameraTuningConfig);

private:
	// 현재 Look 입력값을 저장합니다.
	FVector2D PendingLookInput = FVector2D::ZeroVector;

	// 현재 누적 Aim Yaw를 저장합니다.
	float AccumulatedAimYaw = 0.0f;

	// 현재 누적 Aim Pitch를 저장합니다.
	float AccumulatedAimPitch = 0.0f;

	// 현재 Arm 길이 보간 상태를 저장합니다.
	float CurrentArmLength = 0.0f;

	// 현재 FOV 보간 상태를 저장합니다.
	float CurrentFOV = 0.0f;

	// 외부 시스템이 임시로 주입한 Aim Profile입니다.
	FCFVehicleCameraAimProfile AimProfileOverride;

	// 임시 Aim Profile Override 사용 여부입니다.
	bool bUseAimProfileOverride = false;

	// 카메라 런타임 초기화 완료 여부입니다.
	bool bCameraRuntimeReady = false;

	// 차량 Pawn 캐시입니다.
	TWeakObjectPtr<ACFVehiclePawn> CachedVehiclePawn;
};
