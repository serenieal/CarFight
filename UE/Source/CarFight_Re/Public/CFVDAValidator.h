// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-06-25
// Description: Vehicle DA 입력 보조용 읽기 전용 검증 헬퍼입니다.
// Scope: CFVehicleData 필수 참조, 소켓, 레이아웃, Movement, WheelVisual, DriveState, 기준 DA 비교 검증 리포트를 제공합니다.
// Changelog:
// - v1.2.0: 하드포인트 위치 슬롯과 선택 캡처 소켓 검증 함수를 추가.
// - v1.0.0: EUW_VDAWizard 연동을 위한 BlueprintCallable 검증 함수와 결과 구조체를 추가.
// Migration:
// - HardpointSlots 빈 배열은 오류가 아니며 Info로만 표시한다.
// - 기존 VehicleData 적용 경로는 변경하지 않는다.
// - 에디터 위젯은 BP 내부 판단 대신 이 헬퍼의 리포트를 표시한다.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CFVDAValidator.generated.h"

class UCFVehicleData;

UENUM(BlueprintType)
enum class ECFVDASeverity : uint8
{
	Pass UMETA(DisplayName="정상 (Pass)"),
	Info UMETA(DisplayName="정보 (Info)"),
	Warning UMETA(DisplayName="경고 (Warning)"),
	Error UMETA(DisplayName="오류 (Error)"),
	Blocked UMETA(DisplayName="보류 (Blocked)")
};

USTRUCT(BlueprintType)
struct FCFVDAValidationItem
{
	GENERATED_BODY()

	// 이 검사 항목의 심각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="심각도 (Severity)", ToolTip="이 검사 항목이 정상, 정보, 경고, 오류, 보류 중 어디에 해당하는지 나타냅니다."))
	ECFVDASeverity Severity = ECFVDASeverity::Pass;

	// 이 검사 항목이 속한 검증 그룹 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="그룹 이름 (GroupName)", ToolTip="RequiredRefs, Socket, Layout, Movement 같은 검증 그룹 이름입니다."))
	FName GroupName = NAME_None;

	// 이 검사 항목이 가리키는 CFVehicleData 내부 필드 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="필드 경로 (FieldPath)", ToolTip="VehicleVisualConfig.ChassisMesh처럼 문제가 있는 데이터 필드 경로입니다."))
	FString FieldPath;

	// 에디터 UI에 표시할 사용자 친화적 필드 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="표시 이름 (DisplayName)", ToolTip="검사 결과 UI에 표시할 한글 중심 필드 이름입니다."))
	FText DisplayName;

	// 이 검사 항목의 문제 또는 상태 설명입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="메시지 (Message)", ToolTip="검사 결과가 무엇을 뜻하는지 설명하는 문장입니다."))
	FText Message;

	// 사용자가 다음에 수행하면 좋은 권장 조치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="권장 조치 (RecommendedAction)", ToolTip="이 결과를 해결하거나 확인하기 위한 다음 작업 안내입니다."))
	FText RecommendedAction;
};

USTRUCT(BlueprintType)
struct FCFVDAValidationReport
{
	GENERATED_BODY()

	// 전체 리포트에서 가장 높은 심각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="전체 상태 (OverallSeverity)", ToolTip="리포트 전체에서 가장 높은 심각도입니다."))
	ECFVDASeverity OverallSeverity = ECFVDASeverity::Pass;

	// 검사 대상 차량 DA 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="대상 DA 이름 (TargetDAName)", ToolTip="검사한 대상 차량 DataAsset 이름입니다."))
	FString TargetDAName;

	// 비교 기준 차량 DA 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="기준 DA 이름 (SourceDAName)", ToolTip="비교 기준으로 사용한 차량 DataAsset 이름입니다."))
	FString SourceDAName;

	// Error 심각도 항목 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="오류 수 (ErrorCount)", ToolTip="테스트 전 수정해야 하는 오류 항목 개수입니다."))
	int32 ErrorCount = 0;

	// Warning 심각도 항목 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="경고 수 (WarningCount)", ToolTip="진행 가능하지만 의도 확인이 필요한 경고 항목 개수입니다."))
	int32 WarningCount = 0;

	// Info 심각도 항목 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="정보 수 (InfoCount)", ToolTip="참고용 정보 항목 개수입니다."))
	int32 InfoCount = 0;

	// Blocked 심각도 항목 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="보류 수 (BlockedCount)", ToolTip="선행 조건 부족 또는 툴 범위 밖이라 검사하지 못한 항목 개수입니다."))
	int32 BlockedCount = 0;

	// 전체 검사 결과 항목 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="검사 결과 목록 (Items)", ToolTip="오류, 경고, 정보, 보류를 포함한 세부 검사 결과 목록입니다."))
	TArray<FCFVDAValidationItem> Items;

	// UI에 짧게 표시할 검사 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="요약 문자열 (SummaryText)", ToolTip="검사 결과를 한 줄로 요약한 문자열입니다."))
	FString SummaryText;
};

UCLASS()
class CARFIGHT_RE_API UCFVDAValidator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 대상 VehicleData를 전체 검사하고 기준 VehicleData가 있으면 주요 차이도 함께 비교합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="차량 DA 전체 검사 (Validate Vehicle Data)", ToolTip="대상 차량 DA의 필수 참조, 소켓, 레이아웃, Movement, WheelVisual, DriveState 값을 검사하고 기준 DA와의 주요 차이를 리포트로 반환합니다."))
	static FCFVDAValidationReport ValidateVehicleData(UCFVehicleData* TargetVehicleData, UCFVehicleData* SourceVehicleData);

	// 대상 VehicleData의 필수 메쉬와 Wheel Class 참조 누락 여부를 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="필수 참조 검사 (Validate Required References)", ToolTip="대상 차량 DA에서 차체, 휠 메쉬, 전륜/후륜 Wheel Class 참조가 채워졌는지 검사합니다."))
	static FCFVDAValidationReport ValidateRequiredReferences(UCFVehicleData* TargetVehicleData);

	// 대상 VehicleData의 차체 Static Mesh에 필요한 휠 앵커 소켓이 있는지 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="휠 소켓 검사 (Validate Wheel Sockets)", ToolTip="대상 차량 DA의 차체 메쉬에 Wheel_Anchor 계열 휠 앵커 소켓이 존재하는지 검사합니다."))
	static FCFVDAValidationReport ValidateWheelSockets(UCFVehicleData* TargetVehicleData);

	// 대상 VehicleData의 휠 레이아웃 덮어쓰기와 앵커 값 상태를 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="레이아웃 설정 검사 (Validate Layout Config)", ToolTip="대상 차량 DA의 휠 레이아웃 덮어쓰기 사용 여부와 네 휠 앵커 값이 테스트 가능한 상태인지 검사합니다."))
	static FCFVDAValidationReport ValidateLayoutConfig(UCFVehicleData* TargetVehicleData);

	// 대상 VehicleData의 하드포인트 위치 슬롯과 선택 캡처 소켓 상태를 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="하드포인트 슬롯 검사 (Validate Hardpoint Slots)", ToolTip="대상 차량 DA의 HardpointSlots에서 슬롯 ID, SocketName prefix, 선택 소켓 존재 여부, LocalTransform 상태를 검사합니다. 하드포인트 소켓 누락은 전체 오류가 아니라 경고로만 표시합니다."))
	static FCFVDAValidationReport ValidateHardpointSlots(UCFVehicleData* TargetVehicleData);

	// 대상 VehicleData의 Movement 수치 기본 범위와 위험값을 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="Movement 설정 검사 (Validate Movement Config)", ToolTip="대상 차량 DA의 VehicleMovementConfig 값 중 테스트 전 확인해야 할 위험 수치와 기본 범위 오류를 검사합니다."))
	static FCFVDAValidationReport ValidateMovementConfig(UCFVehicleData* TargetVehicleData);

	// 대상 VehicleData의 WheelVisual 예상 휠 개수와 조향 전륜 개수를 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="WheelVisual 설정 검사 (Validate Wheel Visual Config)", ToolTip="대상 차량 DA의 WheelVisualConfig에서 예상 휠 개수와 조향 전륜 개수가 유효한지 검사합니다."))
	static FCFVDAValidationReport ValidateWheelVisualConfig(UCFVehicleData* TargetVehicleData);

	// 대상 VehicleData의 DriveState 판정 임계값과 유지 시간 기본 범위를 검사합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="DriveState 설정 검사 (Validate Drive State Config)", ToolTip="대상 차량 DA의 DriveStateConfig에서 상태 전환 임계값과 최소 유지 시간이 테스트 가능한 범위인지 검사합니다."))
	static FCFVDAValidationReport ValidateDriveStateConfig(UCFVehicleData* TargetVehicleData);

	// 기준 VehicleData와 대상 VehicleData의 주요 튜닝값 차이를 비교합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle DA Validator", meta=(DisplayName="기준 DA와 비교 (Compare Vehicle Data)", ToolTip="기준 차량 DA와 대상 차량 DA의 주요 튜닝값 차이를 비교해 정보 또는 경고 항목으로 반환합니다."))
	static FCFVDAValidationReport CompareVehicleData(UCFVehicleData* TargetVehicleData, UCFVehicleData* SourceVehicleData);
};
