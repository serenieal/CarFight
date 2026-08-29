// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.30.0
// Date: 2026-08-28
// Description: CF-FQ-040 WSA-P0-03 WheelVisual FL runtime fallback 계약을 명시
// Scope: 차량 시각 자산, Wheel Class 참조, VehicleMovement/WheelVisual/Layout, 피팅 질량, 최대 체력과 선택적 방어 설정을 함께 다룹니다.
// Changelog:
// - v1.30.0: WheelMeshFR/RL/RR이 비어 있으면 런타임에서 WheelMeshFL을 재사용하는 실제 fallback 계약을 Tooltip에 명시.
// - v1.29.0: FCFWheelAnchorPose.RelativeScale과 WheelVisualConfig.bUseWheelSocketScale을 additive 추가. 기존 자산은 OneVector/false 기본값으로 기존 시각·물리 동작을 유지.
// - v1.28.0: ChassisWidth와 UE 5.8 FVehicleTransmissionConfig 대응 typed Transmission fields를 additive 추가. ReverseGearRatios는 positive magnitude 저장 계약을 사용.
// - v1.27.0: EngineMaxRPM과 독립된 HUD Tachometer 레드라인 시작점 RedlineStartRPM을 additive 추가. 0은 미설정이며 EngineMaxRPM/변속값에서 자동 추정하지 않음.
// - v1.26.0: bUseMovementOverrides가 전체 Movement on/off가 아니라 Wheel Runtime 상세 튜닝·ThrottleInputScale 경로 제어임을 Tooltip과 Migration에 명확히 기록.
// - v1.25.0: CF-FQ-034 FIT-P0-02 BaseVehicleMassKg와 MaximumGrossMassKg 데이터 계약을 추가.
// - v1.24.0: CF-FQ-033 DR-P0-01 VehicleDefenseData 선택 참조를 추가하고 기존 None Fallback을 유지.
// - v1.23.0: 차량별 파괴 FX 위치를 SM_Body 소켓으로 지정하는 DestroyedFxSocketName을 추가.
// - v1.22.0: 최소 Damage Runtime에서 사용할 VehicleDurabilityConfig.MaxHealth 설정을 추가.
// - v1.21.0: 자동 스케일된 휠 메시의 바운드 중심을 Wheel_Mesh 원점에 맞추는 중심 보정 옵션을 추가.
// - v1.20.0: WheelRadius 기준으로 휠 StaticMesh 표시 크기를 자동 보정하는 WheelVisual 옵션과 측정 모드를 추가.
// - v1.19.0: 전투 FireOrigin P0 검증을 위해 HardpointSlots를 참조하는 MountProfiles 배열을 추가.
// - v1.18.1: DA_PoliceCar 폐기 결정에 맞춰 현재 P0 DataAsset 기준 마이그레이션 문구로 갱신.
// - v1.18.0: 하드포인트 위치 슬롯 데이터와 차체 소켓 기반 선택 캡처 입력을 추가.
// - v1.17.0: 주행감 Quick Tune의 가속 체감 보장을 위한 ThrottleInputScale 값을 추가.
// - v1.16.0: 레벨 배치 액터 없이 VehicleData에서 차체 메시 소켓을 직접 캡처하는 에디터 버튼 추가.
// - v1.15.0: 차체 메시 소켓에서 휠 앵커 레이아웃을 캡처할 수 있도록 BodyWheelSocketFL/FR/RL/RR 이름 설정을 추가.
// - v1.14.0: VehicleLayoutConfig와 WheelAnchor 포즈 구조를 추가해 차량별 시각 휠 기준 위치를 DataAsset에서 관리.
// - v1.13.0: VehicleMovement 기본값 재정렬 및 레거시 실험값 자동 마이그레이션 추가.
// Migration:
// - v1.29.0 이전 VehicleData는 WheelAnchor RelativeScale=OneVector, bUseWheelSocketScale=false 기본값으로 기존 Wheel_Anchor/Wheel_Mesh 동작과 Legacy AutoScale 정책을 그대로 유지한다.
// - v1.28.0 기존 VehicleData는 UE 5.8 Source Build 기본값과 같은 ChassisWidth=180, Automatic/AutoReverse=true, FinalRatio=3.08, Forward=[2.85,2.02,1.35,1.0], Reverse=[2.86], Up=4500, Down=2000, GearTime=0.4, Efficiency=0.9를 사용하므로 새 필드 부재만으로 기존 주행 결과를 바꾸지 않는다.
// - ReverseGearRatios는 방향 부호가 아닌 positive magnitude만 저장한다. UE 5.8 GetGearRatio()가 reverse 방향에서 음수 부호를 적용하므로 raw 배열에 음수를 저장하지 않는다.
// - v1.27.0 기존 VehicleData는 RedlineStartRPM=0 기본값으로 주행 물리 결과를 그대로 유지한다. 0은 HUD Redline 미설정이며 EngineMaxRPM이나 변속 RPM에서 자동 보정/추정하지 않는다.
// - RedlineStartRPM을 명시하는 차량은 EngineIdleRPM보다 크고 EngineMaxRPM보다 작은 실제 차량별 값을 작성한다.
// - bUseMovementOverrides=false여도 Engine/Drag/Differential/Steering 본체 VehicleData 값은 적용된다. false는 세부 Wheel Runtime Tuning과 ThrottleInputScale이 fallback을 쓰는 현재 계약이며 동작 변경은 없다.
// - 기존 VehicleData는 BaseVehicleMassKg=0과 MaximumGrossMassKg=0 기본값으로 현재 주행 결과를 유지한다.
// - 피팅에 연결할 VehicleData만 실제 기준 질량과 최대 허용 총중량을 명시하며 값을 자동 추정하지 않는다.
// - 기존 VehicleData는 DefaultDefenseData=None 기본값으로 기존 BaseDamage 직접 Health 적용 경로를 유지한다.
// - 방어 런타임을 사용할 차량만 DefaultDefenseData에 UCFVehicleDefenseData를 명시적으로 연결한다.
// - 기존 VehicleData는 DestroyedFxSocketName 기본값 FX_Destroyed를 사용하며 소켓이 없으면 SM_Body Bounds 중심으로 fallback한다.
// - 기존 VehicleData는 bAutoScaleWheelMeshToRadius=false 기본값으로 이전 외형 스케일을 유지한다.
// - 자동 휠 메시 스케일을 쓸 차량만 DA의 WheelVisualConfig에서 옵션을 명시적으로 켠다.
// - 자동 스케일 사용 차량은 기본적으로 메시 바운드 중심도 Wheel_Mesh 원점에 맞춘다. 기존 수동 위치를 유지해야 하면 bAutoCenterWheelMeshBoundsToOrigin=false로 끈다.
// - MountProfiles는 기존 HardpointSlots를 대체하지 않고 LocationSlotRef로 참조한다.
// - Top_01 하드포인트가 있는 기존 자산은 PostLoad에서 RoofTurret_MediumOrLarge 기본 프로파일을 1회 보강한다.
// - 기존 차량 DataAsset의 HardpointSlots는 빈 배열로 시작하며, 빈 배열은 오류가 아니다.
// - 하드포인트 SocketName은 선택 캡처 입력이며, 비어 있으면 기존 LocalTransform을 유지한다.
// - 기존 VehicleMovementConfig는 ThrottleInputScale=1.0 기본값으로 기존 스로틀 입력 체감을 유지한다.
// - 스폰 차량은 DA_*를 열고 "차체 소켓에서 차량 레이아웃 캡처" 버튼을 눌러 레이아웃 값을 생성한다.
// - 새 소켓 이름 필드는 기본 Wheel_Anchor_FL/FR/RL/RR을 사용하므로 기존 자산 동작을 바꾸지 않는다.
// - 기존 자산은 bUseLayoutOverrides=false 기본값으로 BP 수동 Wheel_Anchor 배치를 유지한다.
// - DA_TestSedan, DA_TestSUV 같은 현재 P0 DataAsset만 bUseLayoutOverrides=true와 네 앵커 좌표를 입력한다.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/DataAsset.h"
#include "CFVehicleDriveStateConfig.h"
#include "CFVehicleWeaponTypes.h"
#include "CFVehicleData.generated.h"

class UChaosVehicleWheel;
class UCFCombatFxData;
class UCFVehicleDefenseData;
class UStaticMesh;

UENUM(BlueprintType)
enum class ECFWheelMeshRadiusMeasureMode : uint8
{
	AutoMaxXZ UMETA(DisplayName="자동 XZ 최대값 (AutoMaxXZ)", ToolTip="차축이 Y축인 일반 휠 메시 기준으로 X/Z 바운드 중 큰 값을 반지름으로 사용합니다."),
	AxisX UMETA(DisplayName="X축 반지름 (AxisX)", ToolTip="StaticMesh 로컬 바운드의 X축 Extent를 휠 반지름으로 사용합니다."),
	AxisY UMETA(DisplayName="Y축 반지름 (AxisY)", ToolTip="StaticMesh 로컬 바운드의 Y축 Extent를 휠 반지름으로 사용합니다."),
	AxisZ UMETA(DisplayName="Z축 반지름 (AxisZ)", ToolTip="StaticMesh 로컬 바운드의 Z축 Extent를 휠 반지름으로 사용합니다.")
};

USTRUCT(BlueprintType)
struct FCFVehicleVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 메쉬 (ChassisMesh)", ToolTip="차량 차체 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> ChassisMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="앞왼쪽 휠 메쉬 (WheelMeshFL)", ToolTip="앞왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> WheelMeshFL = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="앞오른쪽 휠 메쉬 (WheelMeshFR)", ToolTip="앞오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 런타임에서 WheelMeshFL을 재사용합니다."))
	TObjectPtr<UStaticMesh> WheelMeshFR = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="뒤왼쪽 휠 메쉬 (WheelMeshRL)", ToolTip="뒤왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 런타임에서 WheelMeshFL을 재사용합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRL = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="뒤오른쪽 휠 메쉬 (WheelMeshRR)", ToolTip="뒤오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 런타임에서 WheelMeshFL을 재사용합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRR = nullptr;
};

USTRUCT(BlueprintType)
struct FCFWheelAnchorPose
{
	GENERATED_BODY()

	// [v1.14.0] Wheel_Anchor_* 컴포넌트에 적용할 부모 기준 상대 위치입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(DisplayName="상대 위치 (RelativeLocation)", ToolTip="Wheel_Anchor_* 컴포넌트에 적용할 부모 기준 상대 위치입니다."))
	FVector RelativeLocation = FVector::ZeroVector;

	// [v1.14.0] Wheel_Anchor_* 컴포넌트에 적용할 부모 기준 상대 회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(DisplayName="상대 회전 (RelativeRotation)", ToolTip="Wheel_Anchor_* 컴포넌트에 적용할 부모 기준 상대 회전입니다."))
	FRotator RelativeRotation = FRotator::ZeroRotator;

	// [v1.29.0] 차체 Wheel Socket에 USER가 작성한 차량별 타이어 크기 Scale입니다. 기존 자산은 OneVector로 호환됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(DisplayName="상대 스케일 (RelativeScale)", ToolTip="차체 Wheel_Anchor_* Socket에 USER가 작성한 타이어 크기 Scale입니다. X/Z는 직경 배율, Y는 폭 배율로 사용하며 기존 자산은 1/1/1을 유지합니다."))
	FVector RelativeScale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FCFVehicleHardpointSlot
{
	GENERATED_BODY()

	// [v1.18.0] 전투 규칙에서 참조할 실제 하드포인트 위치 슬롯 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Hardpoint", meta=(DisplayName="위치 슬롯 ID (LocationSlotId)", ToolTip="전투 규칙에서 참조할 실제 하드포인트 위치 슬롯 ID입니다. 예: Front_01, Top_01"))
	FName LocationSlotId = NAME_None;

	// [v1.18.0] Front, Top 같은 하드포인트 위치 분류입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Hardpoint", meta=(DisplayName="위치 분류 (LocationCategory)", ToolTip="하드포인트 위치를 분류하기 위한 값입니다. 예: Front, Back, LeftSide, RightSide, Top"))
	FName LocationCategory = NAME_None;

	// [v1.18.0] 차체 StaticMesh 소켓에서 이 슬롯 위치를 캡처할 때 사용할 선택 입력 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Hardpoint|Socket", meta=(DisplayName="캡처 소켓 이름 (SocketName)", ToolTip="이 슬롯의 위치를 차체 StaticMesh 소켓에서 캡처할 때 사용할 이름입니다. 비어 있으면 캡처하지 않고 LocalTransform 값을 유지합니다. 예: HP_Front_01, HP_Top_01"))
	FName SocketName = NAME_None;

	// [v1.18.0] 차량 / 차체 기준 하드포인트 로컬 위치입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Hardpoint", meta=(DisplayName="로컬 위치 (LocalLocation)", ToolTip="차량 또는 차체 기준으로 저장된 하드포인트 위치입니다. 런타임 Fire / Aim은 이 값을 원본으로 사용합니다."))
	FVector LocalLocation = FVector::ZeroVector;

	// [v1.18.0] 차량 / 차체 기준 하드포인트 로컬 회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Hardpoint", meta=(DisplayName="로컬 회전 (LocalRotation)", ToolTip="차량 또는 차체 기준으로 저장된 하드포인트 방향입니다. 런타임 Fire / Aim은 이 값을 원본으로 사용합니다."))
	FRotator LocalRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FCFVehicleLayoutConfig
{
	GENERATED_BODY()

	// [v1.14.0] True이면 VehicleData의 바퀴 앵커 기준 위치/회전을 BP 컴포넌트에 적용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(DisplayName="레이아웃 덮어쓰기 사용 (bUseLayoutOverrides)", ToolTip="True이면 VehicleData의 바퀴 앵커 기준 위치와 회전을 BP의 Wheel_Anchor_* 컴포넌트에 적용합니다. False이면 기존 BP 수동 배치를 유지합니다."))
	bool bUseLayoutOverrides = false;

	// [v1.15.0] 차체 메시에서 앞왼쪽 바퀴 중심을 읽을 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout|Socket", meta=(DisplayName="앞왼쪽 차체 소켓 (BodyWheelSocketFL)", ToolTip="차체 Static Mesh에서 앞왼쪽 바퀴 중심 위치를 읽을 소켓 이름입니다. 비어 있으면 Wheel_Anchor_FL을 사용합니다."))
	FName BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");

	// [v1.15.0] 차체 메시에서 앞오른쪽 바퀴 중심을 읽을 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout|Socket", meta=(DisplayName="앞오른쪽 차체 소켓 (BodyWheelSocketFR)", ToolTip="차체 Static Mesh에서 앞오른쪽 바퀴 중심 위치를 읽을 소켓 이름입니다. 비어 있으면 Wheel_Anchor_FR을 사용합니다."))
	FName BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");

	// [v1.15.0] 차체 메시에서 뒤왼쪽 바퀴 중심을 읽을 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout|Socket", meta=(DisplayName="뒤왼쪽 차체 소켓 (BodyWheelSocketRL)", ToolTip="차체 Static Mesh에서 뒤왼쪽 바퀴 중심 위치를 읽을 소켓 이름입니다. 비어 있으면 Wheel_Anchor_RL을 사용합니다."))
	FName BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");

	// [v1.15.0] 차체 메시에서 뒤오른쪽 바퀴 중심을 읽을 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout|Socket", meta=(DisplayName="뒤오른쪽 차체 소켓 (BodyWheelSocketRR)", ToolTip="차체 Static Mesh에서 뒤오른쪽 바퀴 중심 위치를 읽을 소켓 이름입니다. 비어 있으면 Wheel_Anchor_RR을 사용합니다."))
	FName BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");

	// [v1.14.0] 앞왼쪽 바퀴 앵커에 적용할 기준 위치/회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(EditCondition="bUseLayoutOverrides", EditConditionHides, DisplayName="앞왼쪽 바퀴 앵커 (WheelAnchorFL)", ToolTip="Wheel_Anchor_FL 컴포넌트에 적용할 기준 위치와 회전입니다."))
	FCFWheelAnchorPose WheelAnchorFL;

	// [v1.14.0] 앞오른쪽 바퀴 앵커에 적용할 기준 위치/회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(EditCondition="bUseLayoutOverrides", EditConditionHides, DisplayName="앞오른쪽 바퀴 앵커 (WheelAnchorFR)", ToolTip="Wheel_Anchor_FR 컴포넌트에 적용할 기준 위치와 회전입니다."))
	FCFWheelAnchorPose WheelAnchorFR;

	// [v1.14.0] 뒤왼쪽 바퀴 앵커에 적용할 기준 위치/회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(EditCondition="bUseLayoutOverrides", EditConditionHides, DisplayName="뒤왼쪽 바퀴 앵커 (WheelAnchorRL)", ToolTip="Wheel_Anchor_RL 컴포넌트에 적용할 기준 위치와 회전입니다."))
	FCFWheelAnchorPose WheelAnchorRL;

	// [v1.14.0] 뒤오른쪽 바퀴 앵커에 적용할 기준 위치/회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Layout", meta=(EditCondition="bUseLayoutOverrides", EditConditionHides, DisplayName="뒤오른쪽 바퀴 앵커 (WheelAnchorRR)", ToolTip="Wheel_Anchor_RR 컴포넌트에 적용할 기준 위치와 회전입니다."))
	FCFWheelAnchorPose WheelAnchorRR;
};

USTRUCT(BlueprintType)
struct FCFVehicleTransmissionRatios
{
	GENERATED_BODY()

	// UE 5.8 FVehicleTransmissionConfig 기본 ratio를 additive migration 기본값으로 초기화합니다.
	FCFVehicleTransmissionRatios()
	{
		ForwardGearRatios = { 2.85f, 2.02f, 1.35f, 1.0f };
		ReverseGearRatios = { 2.86f };
	}

	// 전진 기어 순서대로 저장하는 양수 기어비 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(DisplayName="전진 기어비 (ForwardGearRatios)", ToolTip="1단부터 순서대로 저장하는 전진 기어비입니다. 값은 0보다 커야 하며 배열 길이가 전진 기어 수입니다."))
	TArray<float> ForwardGearRatios;

	// 후진 기어 순서대로 positive magnitude만 저장하는 기어비 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(DisplayName="후진 기어비 크기 (ReverseGearRatios)", ToolTip="후진 기어비의 양수 크기만 저장합니다. 음수 부호를 넣지 마세요. UE 5.8 GetGearRatio가 후진 방향 부호를 계산할 때 적용합니다."))
	TArray<float> ReverseGearRatios;
};

USTRUCT(BlueprintType)
struct FCFVehicleMovementConfig
{
	GENERATED_BODY()

			// [v1.26.0] 세부 Wheel Runtime Tuning과 ThrottleInputScale을 차량별 값으로 사용할지 선택합니다. Engine/Drag/Differential/Steering 본체 VehicleData 적용 자체를 끄는 스위치는 아닙니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="VehicleMovement 세부 Override 사용 (bUseMovementOverrides)", ToolTip="True이면 차량별 Wheel Runtime 상세 튜닝과 ThrottleInputScale을 사용합니다. False여도 Engine, Drag, Differential, Steering 같은 VehicleData 본체 Movement 값은 현재 런타임에서 적용됩니다."))
	bool bUseMovementOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="이동 프로필 이름 (MovementProfileName)", ToolTip="현재 단계에서는 실제 수치 대신 식별용 이름 또는 메모성 구분자로 사용합니다."))
	FName MovementProfileName = NAME_None;

	// [v1.17.0] 차량별 가속 체감 조절을 위해 실제 스로틀 입력에 곱할 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="스로틀 입력 배율 (ThrottleInputScale)", ToolTip="차량 Pawn이 DriveComp에 전달하는 스로틀 입력에 곱할 배율입니다. 가속감 Quick Tune의 체감 차이를 보장하기 위한 값입니다."))
	float ThrottleInputScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 최대 조향각 (FrontWheelMaxSteerAngle)", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 조향각(deg)입니다."))
	float FrontWheelMaxSteerAngle = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 최대 브레이크 토크 (FrontWheelMaxBrakeTorque)", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float FrontWheelMaxBrakeTorque = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 최대 브레이크 토크 (RearWheelMaxBrakeTorque)", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float RearWheelMaxBrakeTorque = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 최대 핸드브레이크 토크 (RearWheelMaxHandBrakeTorque)", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 핸드브레이크 토크입니다."))
	float RearWheelMaxHandBrakeTorque = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 휠 반지름 (FrontWheelRadius)", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float FrontWheelRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 휠 반지름 (RearWheelRadius)", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float RearWheelRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 휠 폭 (FrontWheelWidth)", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float FrontWheelWidth = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 휠 폭 (RearWheelWidth)", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float RearWheelWidth = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 마찰력 배수 (FrontWheelFrictionForceMultiplier)", ToolTip="전륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float FrontWheelFrictionForceMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 마찰력 배수 (RearWheelFrictionForceMultiplier)", ToolTip="후륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float RearWheelFrictionForceMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 코너링 강성 (FrontWheelCorneringStiffness)", ToolTip="전륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float FrontWheelCorneringStiffness = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 코너링 강성 (RearWheelCorneringStiffness)", ToolTip="후륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float RearWheelCorneringStiffness = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 Wheel Load Ratio (FrontWheelLoadRatio)", ToolTip="전륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float FrontWheelLoadRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 Wheel Load Ratio (RearWheelLoadRatio)", ToolTip="후륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float RearWheelLoadRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 스프링 강성 (FrontWheelSpringRate)", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float FrontWheelSpringRate = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 스프링 강성 (RearWheelSpringRate)", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float RearWheelSpringRate = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 스프링 프리로드 (FrontWheelSpringPreload)", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float FrontWheelSpringPreload = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 스프링 프리로드 (RearWheelSpringPreload)", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float RearWheelSpringPreload = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 서스펜션 최대 상승 (FrontWheelSuspensionMaxRaise)", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float FrontWheelSuspensionMaxRaise = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 서스펜션 최대 상승 (RearWheelSuspensionMaxRaise)", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float RearWheelSuspensionMaxRaise = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 서스펜션 최대 하강 (FrontWheelSuspensionMaxDrop)", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float FrontWheelSuspensionMaxDrop = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 서스펜션 최대 하강 (RearWheelSuspensionMaxDrop)", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float RearWheelSuspensionMaxDrop = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 엔진 영향 여부 (bFrontWheelAffectedByEngine)", ToolTip="전륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bFrontWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 엔진 영향 여부 (bRearWheelAffectedByEngine)", ToolTip="후륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bRearWheelAffectedByEngine = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Sweep Shape (FrontWheelSweepShape)", ToolTip="전륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape FrontWheelSweepShape = ESweepShape::Raycast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Sweep Shape (RearWheelSweepShape)", ToolTip="후륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape RearWheelSweepShape = ESweepShape::Raycast;

	// [v1.28.0] 공기저항 면적 계산에 사용하는 UE 5.8 ChassisWidth cm 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.01", Units="cm", DisplayName="차체 폭 (ChassisWidth)", ToolTip="VehicleMovementComp의 ChassisWidth 값입니다. UE 5.8은 ChassisWidth×ChassisHeight를 공기저항 면적 계산에 사용합니다."))
	float ChassisWidth = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 높이 (ChassisHeight)", ToolTip="VehicleMovementComp의 ChassisHeight 값입니다."))
	float ChassisHeight = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="공기 저항 계수 (DragCoefficient)", ToolTip="VehicleMovementComp의 DragCoefficient 값입니다."))
	float DragCoefficient = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="다운포스 계수 (DownforceCoefficient)", ToolTip="VehicleMovementComp의 DownforceCoefficient 값입니다."))
	float DownforceCoefficient = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오버라이드 사용 여부 (bEnableCenterOfMassOverride)", ToolTip="VehicleMovementComp의 중심질량 오버라이드 사용 여부입니다."))
	bool bEnableCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오프셋 (CenterOfMassOverride)", ToolTip="VehicleMovementComp의 CenterOfMassOverride 값입니다."))
	FVector CenterOfMassOverride = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 최대 토크 (EngineMaxTorque)", ToolTip="EngineSetup.MaxTorque 값입니다."))
	float EngineMaxTorque = 750.0f;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 최대 RPM (EngineMaxRPM)", ToolTip="EngineSetup.MaxRPM에 실제 적용되는 물리 엔진 최대 RPM입니다. HUD Redline 시작값으로 자동 재해석하지 않습니다."))
	float EngineMaxRPM = 7000.0f;

	// [v1.27.0] HUD Tachometer의 85% Red Zone 시작 위치에 매핑할 차량별 실제 레드라인 시작 RPM입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", Units="rpm", DisplayName="레드라인 시작 RPM (RedlineStartRPM)", ToolTip="HUD Tachometer의 실제 레드라인 시작 RPM입니다. 0은 미설정이며 EngineMaxRPM이나 변속 RPM에서 자동 추정하지 않습니다. 값을 설정하면 EngineIdleRPM보다 크고 EngineMaxRPM보다 작아야 합니다."))
	float RedlineStartRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 아이들 RPM (EngineIdleRPM)", ToolTip="EngineSetup.EngineIdleRPM 값입니다."))
	float EngineIdleRPM = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 브레이크 효과 (EngineBrakeEffect)", ToolTip="EngineSetup.EngineBrakeEffect 값입니다."))
	float EngineBrakeEffect = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 회전 상승 MOI (EngineRevUpMOI)", ToolTip="EngineSetup.EngineRevUpMOI 값입니다."))
	float EngineRevUpMOI = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 회전 하강 속도 (EngineRevDownRate)", ToolTip="EngineSetup.EngineRevDownRate 값입니다."))
	float EngineRevDownRate = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="디퍼렌셜 타입 (DifferentialType)", ToolTip="DifferentialSetup.DifferentialType 값입니다."))
	EVehicleDifferential DifferentialType = EVehicleDifferential::RearWheelDrive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="전후륜 구동 분배 (FrontRearSplit)", ToolTip="DifferentialSetup.FrontRearSplit 값입니다."))
	float FrontRearSplit = 0.5f;

	// [v1.28.0] UE 5.8 TransmissionSetup을 Automatic control mode로 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(DisplayName="자동 변속 사용 (bUseAutomaticGears)", ToolTip="True이면 Chaos TransmissionSetup이 자동 변속 control mode를 사용합니다. 변속기 구조 종류와는 별개입니다."))
	bool bUseAutomaticGears = true;

	// [v1.28.0] 전후진 입력에 따라 Chaos가 자동으로 Reverse 상태를 선택할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(DisplayName="자동 후진 전환 사용 (bUseAutoReverse)", ToolTip="True이면 Chaos의 AutoReverse 동작을 사용합니다."))
	bool bUseAutoReverse = true;

	// [v1.28.0] 전진/후진 기어비 배열을 묶은 typed ratio-set입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(DisplayName="변속기 기어비 (TransmissionRatios)", ToolTip="전진/후진 기어비를 순서대로 보관합니다. 후진 배열도 positive magnitude만 저장합니다."))
	FCFVehicleTransmissionRatios TransmissionRatios;

	// [v1.28.0] 각 기어비에 곱하는 최종 감속비입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(ClampMin="0.0001", DisplayName="최종 감속비 (FinalRatio)", ToolTip="Chaos TransmissionSetup의 FinalRatio입니다."))
	float FinalRatio = 3.08f;

	// [v1.28.0] 자동 변속에서 상향 변속을 요청하는 엔진 RPM 기준입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(ClampMin="0.0", Units="rpm", DisplayName="상향 변속 RPM (ChangeUpRPM)", ToolTip="Chaos 자동 변속의 ChangeUpRPM 기준입니다."))
	float ChangeUpRPM = 4500.0f;

	// [v1.28.0] 자동 변속에서 하향 변속을 요청하는 엔진 RPM 기준입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(ClampMin="0.0", Units="rpm", DisplayName="하향 변속 RPM (ChangeDownRPM)", ToolTip="Chaos 자동 변속의 ChangeDownRPM 기준입니다."))
	float ChangeDownRPM = 2000.0f;

	// [v1.28.0] 한 기어에서 다른 기어로 전환하는 데 걸리는 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(ClampMin="0.0", Units="s", DisplayName="변속 시간 (GearChangeTime)", ToolTip="Chaos TransmissionSetup의 GearChangeTime(초)입니다."))
	float GearChangeTime = 0.4f;

	// [v1.28.0] 변속기 기계 손실을 표현하는 효율 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Transmission", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="변속 효율 (TransmissionEfficiency)", ToolTip="Chaos TransmissionSetup의 TransmissionEfficiency입니다. 1.0은 손실 없음입니다."))
	float TransmissionEfficiency = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="조향 타입 (SteeringType)", ToolTip="SteeringSetup.SteeringType 값입니다."))
	ESteeringType SteeringType = ESteeringType::AngleRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="조향 Angle Ratio (SteeringAngleRatio)", ToolTip="SteeringSetup.AngleRatio 값입니다."))
	float SteeringAngleRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="레거시 휠 마찰 위치 사용 여부 (bLegacyWheelFrictionPosition)", ToolTip="VehicleMovementComp의 bLegacyWheelFrictionPosition 값입니다."))
	bool bLegacyWheelFrictionPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Additional Offset (FrontWheelAdditionalOffset)", ToolTip="전륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector FrontWheelAdditionalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Additional Offset (RearWheelAdditionalOffset)", ToolTip="후륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector RearWheelAdditionalOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCFVehicleWheelVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="WheelVisual 덮어쓰기 사용 (bUseWheelVisualOverrides)", ToolTip="True이면 이 차량이 별도 WheelVisual 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseWheelVisualOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0", DisplayName="예상 휠 개수 (ExpectedWheelCount)", ToolTip="차량 휠 시각 동기화에서 기대하는 바퀴 개수입니다."))
	int32 ExpectedWheelCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0", DisplayName="조향 전륜 개수 (FrontWheelCountForSteering)", ToolTip="차량 휠 시각 동기화에서 조향 대상으로 보는 전륜 개수입니다."))
	int32 FrontWheelCountForSteering = 2;

	// [v1.29.0] True이면 차체 Wheel Socket Scale을 차량별 타이어 크기 Authoring authority로 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 소켓 스케일 사용 (bUseWheelSocketScale)", ToolTip="True이면 차체 Wheel Socket의 Scale을 차량별 타이어 크기 기준으로 사용합니다. USER가 정한 X/Z 직경 배율과 Y 폭 배율을 시각 Wheel Mesh에 적용하고 같은 크기에서 물리 WheelRadius/Width를 파생하는 신규 경로용 명시 flag입니다. WheelRadius 기준 시각 자동 스케일과 동시에 사용하지 않습니다."))
	bool bUseWheelSocketScale = false;

	// [v1.20.0] True이면 WheelMesh의 원본 바운드 반지름을 VehicleMovementConfig의 WheelRadius에 맞춰 표시 스케일을 자동 보정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="WheelRadius 기준 휠 메시 자동 스케일 (bAutoScaleWheelMeshToRadius)", ToolTip="True이면 VehicleVisualConfig의 WheelMesh 바운드 반지름을 측정해서 FrontWheelRadius / RearWheelRadius에 맞게 Wheel_Mesh_* 표시 스케일을 자동 적용합니다. 기존 수동 스케일 보존을 위해 기본값은 False입니다."))
	bool bAutoScaleWheelMeshToRadius = false;

	// [v1.20.0] 휠 StaticMesh 바운드에서 어떤 축을 반지름으로 볼지 정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(EditCondition="bAutoScaleWheelMeshToRadius", EditConditionHides, DisplayName="휠 메시 반지름 측정 모드 (WheelMeshRadiusMeasureMode)", ToolTip="휠 StaticMesh의 로컬 바운드에서 반지름으로 사용할 축입니다. 일반적인 차축 Y 기준 휠은 자동 XZ 최대값을 사용합니다."))
	ECFWheelMeshRadiusMeasureMode WheelMeshRadiusMeasureMode = ECFWheelMeshRadiusMeasureMode::AutoMaxXZ;

	// [v1.21.0] True이면 자동 스케일 후 휠 메시 바운드 중심을 Wheel_Mesh 컴포넌트 원점에 맞춥니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(EditCondition="bAutoScaleWheelMeshToRadius", EditConditionHides, DisplayName="휠 메시 바운드 중심 자동 보정 (bAutoCenterWheelMeshBoundsToOrigin)", ToolTip="True이면 자동 스케일 후 StaticMesh 바운드 중심이 Wheel_Mesh_* 컴포넌트 원점과 일치하도록 RelativeLocation을 보정합니다. 메시 피벗이 휠 중심이 아닐 때 시각 휠과 물리 휠 중심을 맞추는 옵션입니다."))
	bool bAutoCenterWheelMeshBoundsToOrigin = true;

	// [v1.20.0] 자동 계산된 휠 메시 표시 스케일의 최소 허용값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(EditCondition="bAutoScaleWheelMeshToRadius", EditConditionHides, ClampMin="0.01", DisplayName="휠 메시 자동 스케일 최소값 (WheelMeshScaleClampMin)", ToolTip="자동 계산된 휠 메시 표시 스케일이 이 값보다 작아지지 않게 제한합니다. 너무 작은 메시 측정 오류를 막기 위한 안전값입니다."))
	float WheelMeshScaleClampMin = 0.25f;

	// [v1.20.0] 자동 계산된 휠 메시 표시 스케일의 최대 허용값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(EditCondition="bAutoScaleWheelMeshToRadius", EditConditionHides, ClampMin="0.01", DisplayName="휠 메시 자동 스케일 최대값 (WheelMeshScaleClampMax)", ToolTip="자동 계산된 휠 메시 표시 스케일이 이 값보다 커지지 않게 제한합니다. 비정상 바운드나 잘못된 WheelRadius 입력을 빠르게 발견하기 위한 안전값입니다."))
	float WheelMeshScaleClampMax = 4.0f;
};

USTRUCT(BlueprintType)
struct FCFVehicleReferenceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Wheel Class (FrontWheelClass)", ToolTip="차량 전륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> FrontWheelClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Wheel Class (RearWheelClass)", ToolTip="차량 후륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> RearWheelClass;
};

USTRUCT(BlueprintType)
struct FCFVehicleDurabilityConfig
{
	GENERATED_BODY()

	// [v1.22.0] 차량 체력 컴포넌트가 최초 초기화에 사용할 최대 체력입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Durability", meta=(ClampMin="1.0", DisplayName="최대 체력 (MaxHealth)", ToolTip="VehicleHealthComp가 최초 초기화에 사용할 차량 최대 체력입니다. 기존 VehicleData는 C++ 기본값 100을 사용합니다."))
	float MaxHealth = 100.0f;
};

UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 레거시 VehicleMovement 실험값 세트를 현재 프로젝트 기준값으로 보정합니다.
	virtual void PostLoad() override;

#if WITH_EDITOR
	// VehicleVisualConfig.ChassisMesh 소켓에서 휠 앵커와 선택 하드포인트 위치 값을 직접 캡처합니다.
	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|Vehicle Data|Editor", meta=(DisplayName="차체 소켓에서 차량 레이아웃 캡처 (Capture Vehicle Layout From Chassis Sockets)", ToolTip="VehicleVisualConfig.ChassisMesh의 Wheel_Anchor_FL/FR/RL/RR 소켓과 HardpointSlots에 선언된 SocketName을 읽어 차량 레이아웃 값을 기록합니다. 하드포인트 SocketName이 비어 있거나 소켓이 없어도 휠 캡처 성공 자체를 실패로 보지 않습니다."))
	void CaptureLayoutFromChassisSockets();
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 시각 설정 (VehicleVisualConfig)", ToolTip="차체와 휠 시각 자산 참조를 묶은 설정입니다."))
	FCFVehicleVisualConfig VehicleVisualConfig;

	// [v1.14.0] 차량별 Wheel_Anchor_* 기준 위치/회전을 묶은 레이아웃 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 레이아웃 설정 (VehicleLayoutConfig)", ToolTip="차량별 Wheel_Anchor_FL/FR/RL/RR 기준 위치와 회전을 묶은 설정입니다. bUseLayoutOverrides가 꺼져 있으면 기존 BP 수동 배치를 유지합니다."))
	FCFVehicleLayoutConfig VehicleLayoutConfig;

	// [v1.18.0] 차량별 하드포인트 위치 슬롯 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="하드포인트 위치 슬롯 (HardpointSlots)", ToolTip="이 차량이 제공하는 하드포인트 위치 슬롯 목록입니다. 빈 배열은 아직 하드포인트 위치를 선언하지 않은 상태이며 오류가 아닙니다."))
	TArray<FCFVehicleHardpointSlot> HardpointSlots;

		// [v1.19.0] 차량별 전투 장착 프로파일 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전투 장착 프로파일 (MountProfiles)", ToolTip="이 차량의 하드포인트 위치 슬롯에 어떤 전투 장착 규칙을 연결할지 정의합니다. P0에서는 Top_01 + Turret 프로파일부터 사용합니다."))
	TArray<FCFVehicleMountProfile> MountProfiles;

	// [v1.25.0] 장비·탄약·방어를 더하기 전 차량 플랫폼 자체의 기준 질량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Fitting", meta=(ClampMin="0.0", Units="kg", DisplayName="기준 차량 질량 kg (BaseVehicleMassKg)", ToolTip="피팅 질량 계산의 차량 플랫폼 기준값입니다. 0은 미설정이며 기존 주행에는 영향을 주지 않습니다. 피팅에 사용할 차량은 실제 밸런스 기준값을 명시해야 합니다."))
	float BaseVehicleMassKg = 0.0f;

	// [v1.25.0] 장비·탄약·방어를 포함해 이 차량이 허용하는 최대 총중량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Fitting", meta=(ClampMin="0.0", Units="kg", DisplayName="최대 허용 총중량 kg (MaximumGrossMassKg)", ToolTip="피팅 적용을 허용하는 최대 차량 총중량입니다. 0은 미설정이며 BaseVehicleMassKg보다 크거나 같아야 합니다."))
	float MaximumGrossMassKg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 이동 설정 (VehicleMovementConfig)", ToolTip="VehicleMovement 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleMovementConfig VehicleMovementConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="휠 시각 설정 (WheelVisualConfig)", ToolTip="WheelSync 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleWheelVisualConfig WheelVisualConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 참조 설정 (VehicleReferenceConfig)", ToolTip="전륜과 후륜 Wheel Class 등 차량 참조형 자산 설정입니다."))
	FCFVehicleReferenceConfig VehicleReferenceConfig;

				// [v1.22.0] 차량 최대 체력 값을 묶은 내구도 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 내구도 설정 (VehicleDurabilityConfig)", ToolTip="VehicleHealthComp가 초기화할 최대 체력을 제공합니다. MaxHealth는 차량 내구도 최대값으로 계속 유지됩니다."))
	FCFVehicleDurabilityConfig VehicleDurabilityConfig;

	// [v1.24.0] 이 차량이 사용할 쉴드·방향별 장갑 정적 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|Defense", meta=(DisplayName="기본 차량 방어 데이터 (DefaultDefenseData)", ToolTip="쉴드, 재생, 장갑 저항과 6방향 장갑 설정을 제공하는 VehicleDefenseData입니다. 비어 있으면 기존 BaseDamage 직접 Health 적용 경로를 유지합니다."))
	TObjectPtr<UCFVehicleDefenseData> DefaultDefenseData = nullptr;

	// 최초 차량 파괴 전환에서 재생할 기본 Destroyed FX 데이터입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|FX", meta=(DisplayName="기본 파괴 FX 데이터", ToolTip="차량이 최초 파괴 상태로 전환될 때 재생할 CombatFxData입니다. 비어 있어도 파괴 판정은 유지합니다."))
	TObjectPtr<UCFCombatFxData> DefaultDestroyedFxData = nullptr;

	// 차량별 파괴 FX 위치를 제공할 SM_Body StaticMesh 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|FX", meta=(DisplayName="파괴 FX 차체 소켓 이름 (DestroyedFxSocketName)", ToolTip="SM_Body에 적용된 차체 StaticMesh에서 파괴 FX 월드 위치와 회전을 읽을 소켓 이름입니다. 기본 FX_Destroyed를 사용하며, 소켓이 없으면 SM_Body Bounds 중심으로 fallback합니다."))
	FName DestroyedFxSocketName = TEXT("FX_Destroyed");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="DriveState 설정 (DriveStateConfig)", ToolTip="차량별 DriveState 판정 임계값과 히스테리시스 설정입니다."))
	FCFVehicleDriveStateConfig DriveStateConfig;
};
