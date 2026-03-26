// Version: 1.2.0
// Date: 2026-03-18
// Description: CarFight 차량 물리/디버그 유틸리티 라이브러리 헤더

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "CFVehicleDriveComp.h"
#include "CFVehiclePawn.h"
#include "CarFightVehicleUtils.generated.h"

/**
 * CarFight 프로젝트 전용 차량 물리/디버그 유틸리티
 */
UCLASS()
class CARFIGHT_RE_API UCarFightVehicleUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * [v1.0] 물리 엔진의 실제 휠 상태(조향각, 회전각, 서스펜션)를 가져와 트랜스폼으로 반환합니다.
	 * 블루프린트의 GetWheelState가 제공하지 않는 회전 데이터를 C++ 레벨에서 추출합니다.
	 * @param MovementComponent - 대상 차량 무브먼트 컴포넌트
	 * @param WheelIndex - 바퀴 인덱스 (0~3)
	 * @param OutOffset - 서스펜션이 적용된 위치 오프셋 (Z축)
	 * @param OutRotation - 조향(Yaw)과 굴러감(Pitch)이 적용된 회전값
	 */
	UFUNCTION(BlueprintPure, Category="CarFight|Physics", meta=(ToolTip="카오스 비히클의 실제 휠 회전/조향/서스펜션 상태를 읽어 상대 위치와 회전을 반환합니다."))
	static void GetRealWheelTransform(
		UChaosWheeledVehicleMovementComponent* MovementComponent,
		int32 WheelIndex,
		FVector& OutOffset,
		FRotator& OutRotation
	);

	/**
	 * [v1.1] Drive 상태 Enum을 화면/로그 친화 문자열로 변환합니다.
	 * @param InDriveState - 변환할 Drive 상태 Enum
	 * @return 사람이 읽기 쉬운 Drive 상태 이름 문자열
	 */
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Drive 상태 Enum을 화면/로그 출력용 문자열로 변환합니다."))
	static FString ConvDriveStateToDisplayString(ECFVehicleDriveState InDriveState);

	/**
	 * [v1.1] Drive 상태 스냅샷을 한 줄 디버그 문자열로 변환합니다.
	 * @param InDriveStateSnapshot - 변환할 Drive 상태 스냅샷
	 * @return 속도, 전방 속도, 접지 여부, 입력 상태가 포함된 디버그 문자열
	 */
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Drive 상태 스냅샷을 화면/로그 출력용 한 줄 문자열로 변환합니다."))
	static FString MakeDriveStateSnapshotDebugString(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot);

	/**
	 * [v1.2] Drive 상태 스냅샷을 멀티라인 디버그 문자열로 변환합니다.
	 * @param InDriveStateSnapshot - 변환할 Drive 상태 스냅샷
	 * @param bIncludeInputState - True이면 입력 상태 라인을 추가합니다.
	 * @return 줄바꿈이 포함된 Drive 상태 멀티라인 디버그 문자열
	 */
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Drive 상태 스냅샷을 줄바꿈이 포함된 멀티라인 디버그 문자열로 변환합니다."))
	static FString MakeDriveStateSnapshotMultilineDebugString(
		const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot,
		bool bIncludeInputState = true
	);

	/**
	 * [v1.1] Pawn 차량 디버그 스냅샷을 한 줄 또는 확장 문자열로 변환합니다.
	 * @param InVehicleDebugSnapshot - 변환할 Pawn 차량 디버그 스냅샷
	 * @param bIncludeRuntimeSummary - True이면 RuntimeSummary를 문자열 끝에 추가합니다.
	 * @param bIncludeTransitionSummary - True이면 마지막 상태 전이 요약 문자열을 포함합니다.
	 * @param bIncludeInputState - True이면 입력 상태(Throttle/Steering/Brake/Handbrake)를 포함합니다.
	 * @return 화면/로그/UMG 텍스트에 바로 쓸 수 있는 디버그 문자열
	 */
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Pawn 차량 디버그 스냅샷을 화면/로그/UMG 텍스트에 바로 쓸 수 있는 문자열로 변환합니다."))
	static FString MakeVehicleDebugSnapshotDebugString(
		const FCFVehicleDebugSnapshot& InVehicleDebugSnapshot,
		bool bIncludeRuntimeSummary = false,
		bool bIncludeTransitionSummary = true,
		bool bIncludeInputState = true
	);

	/**
	 * [v1.2] Pawn 차량 디버그 스냅샷을 멀티라인 디버그 문자열로 변환합니다.
	 * @param InVehicleDebugSnapshot - 변환할 Pawn 차량 디버그 스냅샷
	 * @param bIncludeRuntimeSummary - True이면 RuntimeSummary 라인을 추가합니다.
	 * @param bIncludeTransitionSummary - True이면 상태 전이 라인을 추가합니다.
	 * @param bIncludeInputState - True이면 입력 상태 라인을 추가합니다.
	 * @return 줄바꿈이 포함된 Pawn 차량 멀티라인 디버그 문자열
	 */
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle|Debug", meta=(ToolTip="Pawn 차량 디버그 스냅샷을 줄바꿈이 포함된 멀티라인 디버그 문자열로 변환합니다."))
	static FString MakeVehicleDebugSnapshotMultilineDebugString(
		const FCFVehicleDebugSnapshot& InVehicleDebugSnapshot,
		bool bIncludeRuntimeSummary = true,
		bool bIncludeTransitionSummary = true,
		bool bIncludeInputState = true
	);
};
