// Version: 1.0.0
// Date: 2026-02-04
// Description: 카오스 비히클의 숨겨진 휠 트랜스폼(회전, 조향) 데이터를 추출하는 라이브러리 헤더

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChaosWheeledVehicleMovementComponent.h" 
#include "CarFightVehicleUtils.generated.h"

/**
 * CarFight 프로젝트 전용 차량 물리 유틸리티
 */
UCLASS()
class CARFIGHT_RE_API UCarFightVehicleUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * [v1.0] 물리 엔진의 실제 휠 상태(조향각, 회전각, 서스펜션)를 가져와 트랜스폼으로 반환합니다.
	 * 블루프린트의 GetWheelState가 제공하지 않는 회전 데이터를 C++ 레벨에서 추출합니다.
	 * * @param MovementComponent - 대상 차량 무브먼트 컴포넌트
	 * @param WheelIndex - 바퀴 인덱스 (0~3)
	 * @param OutOffset - 서스펜션이 적용된 위치 오프셋 (Z축)
	 * @param OutRotation - 조향(Yaw)과 굴러감(Pitch)이 적용된 회전값
	 */
	UFUNCTION(BlueprintPure, Category = "CarFight|Physics")
	static void GetRealWheelTransform(
		UChaosWheeledVehicleMovementComponent* MovementComponent,
		int32 WheelIndex,
		FVector& OutOffset,
		FRotator& OutRotation
	);
};