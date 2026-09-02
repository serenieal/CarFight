// Version: 1.3.0
// Date: 2026-08-29
// Description: CarFight 차량 디버그 유틸리티 라이브러리 헤더
// Changelog:
// - v1.3.0: 사용 중단된 GetRealWheelTransform legacy reflection API를 제거했습니다.
// Migration:
// - WheelSync runtime은 UCFWheelSyncComp의 정식 Chaos wheel 경로를 사용합니다. GetRealWheelTransform 대체 호출은 필요하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
