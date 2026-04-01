#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFWheelSyncComp.generated.h"

class UChaosWheeledVehicleMovementComponent;
class USceneComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FCFWheelVisualInput
{
	GENERATED_BODY()

	// 이 입력이 대응하는 휠 인덱스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 인덱스 (WheelIndex)", ToolTip="이 입력이 대응하는 휠 인덱스입니다. INDEX_NONE이면 아직 유효한 휠이 연결되지 않은 상태입니다."))
	int32 WheelIndex = INDEX_NONE;

	// 현재 프레임 입력이 유효한지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="입력 유효 여부 (bIsValidInput)", ToolTip="현재 프레임의 Wheel Visual 입력이 유효한지 여부입니다. False이면 해당 휠 입력은 적용 대상에서 제외할 수 있습니다."))
	bool bIsValidInput = false;

	// Anchor에 적용할 조향 Yaw 각도(deg)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="조향 Yaw 각도 (SteeringYawDeg)", ToolTip="Anchor에 적용할 조향 Yaw 각도(deg)입니다."))
	float SteeringYawDeg = 0.0f;

	// Mesh에 반영할 누적 SpinPitch 각도(deg)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="누적 스핀 Pitch 각도 (SpinPitchDeg)", ToolTip="Mesh에 반영할 누적 SpinPitch 각도(deg)입니다."))
	float SpinPitchDeg = 0.0f;

	// Mesh에 이번 프레임 추가로 반영할 SpinPitch delta 각도(deg)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="프레임 스핀 Pitch 델타 (SpinPitchDeltaDeg)", ToolTip="Mesh에 이번 프레임 추가로 반영할 SpinPitch delta 각도(deg)입니다."))
	float SpinPitchDeltaDeg = 0.0f;

	// SpinPitch를 절대각 대신 delta 회전으로 적용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="스핀 Pitch 델타 적용 (bApplySpinPitchAsDelta)", ToolTip="SpinPitch를 절대각 대신 delta 회전으로 적용할지 여부입니다."))
	bool bApplySpinPitchAsDelta = false;

	// Anchor에 적용할 서스펜션 Z 오프셋입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="서스펜션 Z 오프셋 (SuspensionOffsetZ)", ToolTip="Anchor에 적용할 서스펜션 Z 오프셋입니다."))
	float SuspensionOffsetZ = 0.0f;
};

USTRUCT(BlueprintType)
struct FCFWheelVisualState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="휠 인덱스 (WheelIndex)", ToolTip="이 시각 상태가 대응하는 휠 인덱스입니다."))
	int32 WheelIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="적용 성공 여부 (bApplied)", ToolTip="마지막 Wheel Visual 상태 적용이 성공했는지 여부입니다."))
	bool bApplied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="마지막 Anchor 위치 (LastAppliedAnchorLocation)", ToolTip="마지막으로 Anchor에 적용한 로컬 위치입니다."))
	FVector LastAppliedAnchorLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="마지막 Anchor 회전 (LastAppliedAnchorRotation)", ToolTip="마지막으로 Anchor에 적용한 로컬 회전입니다."))
	FRotator LastAppliedAnchorRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="마지막 휠 메쉬 회전 (LastAppliedMeshRotation)", ToolTip="마지막으로 휠 메쉬에 적용한 로컬 회전입니다."))
	FRotator LastAppliedMeshRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FCFWheelHelperCompareState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="휠 인덱스 (WheelIndex)", ToolTip="비교 상태가 대응하는 휠 인덱스입니다."))
	int32 WheelIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="Helper 샘플 시도 여부 (bHelperSampleAttempted)", ToolTip="현재 프레임에 helper 샘플 추출을 시도했는지 여부입니다."))
	bool bHelperSampleAttempted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="디버그 조향 Yaw (DebugSteeringYawDeg)", ToolTip="디버그 파이프 기준 조향 Yaw 각도(deg)입니다."))
	float DebugSteeringYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="Helper 조향 Yaw (HelperSteeringYawDeg)", ToolTip="helper 기준 조향 Yaw 각도(deg)입니다."))
	float HelperSteeringYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="조향 Yaw 차이 (DeltaSteeringYawDeg)", ToolTip="디버그 파이프와 helper 사이의 조향 Yaw 차이(deg)입니다."))
	float DeltaSteeringYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="디버그 스핀 Pitch (DebugSpinPitchDeg)", ToolTip="디버그 파이프 기준 스핀 Pitch 각도(deg)입니다."))
	float DebugSpinPitchDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="Helper 스핀 Pitch (HelperSpinPitchDeg)", ToolTip="helper 기준 스핀 Pitch 각도(deg)입니다."))
	float HelperSpinPitchDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="스핀 Pitch 차이 (DeltaSpinPitchDeg)", ToolTip="디버그 파이프와 helper 사이의 스핀 Pitch 차이(deg)입니다."))
	float DeltaSpinPitchDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="디버그 서스펜션 Z (DebugSuspensionOffsetZ)", ToolTip="디버그 파이프 기준 서스펜션 Z 오프셋입니다."))
	float DebugSuspensionOffsetZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="Helper 서스펜션 Z (HelperSuspensionOffsetZ)", ToolTip="helper 기준 서스펜션 Z 오프셋입니다."))
	float HelperSuspensionOffsetZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="서스펜션 Z 차이 (DeltaSuspensionOffsetZ)", ToolTip="디버그 파이프와 helper 사이의 서스펜션 Z 차이입니다."))
	float DeltaSuspensionOffsetZ = 0.0f;
};

UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFWheelSyncComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCFWheelSyncComp();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="예상 휠 개수 (ExpectedWheelCount)", ToolTip="WheelSync가 준비와 검증에 사용할 예상 휠 개수입니다."))
	int32 ExpectedWheelCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="BeginPlay 자동 준비 (bAutoPrepareOnBeginPlay)", ToolTip="True이면 BeginPlay에서 WheelSync 준비 절차를 자동으로 시도합니다."))
	bool bAutoPrepareOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="이름 기반 컴포넌트 자동 탐색 (bAutoFindComponentsByName)", ToolTip="True이면 등록된 컴포넌트 이름을 기준으로 Anchor와 Wheel Mesh를 자동 탐색합니다."))
	bool bAutoFindComponentsByName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug", meta=(InlineEditConditionToggle, DisplayName="디버그 모드 (bDebugMode)", ToolTip="True이면 개발/검증용 상세 옵션을 디테일 패널에 노출합니다."))
	bool bDebugMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="상세 로그 출력 (bVerboseLog)", ToolTip="True이면 WheelSync 상세 로그를 출력합니다."))
	bool bVerboseLog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="C++ 변환 적용 사용 (bEnableApplyTransformsInCpp)", ToolTip="True이면 계산된 Wheel Visual 변환을 C++에서 직접 적용합니다."))
	bool bEnableApplyTransformsInCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="실제 휠 Transform Helper 사용 (bUseRealWheelTransformHelper)", ToolTip="True이면 helper 입력 경로를 사용합니다."))
	bool bUseRealWheelTransformHelper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper가 디버그 입력 전체 덮어쓰기 (bHelperOverridesDebugInputs)", ToolTip="True이면 helper 결과가 디버그 파이프 입력 전체를 덮어씁니다."))
	bool bHelperOverridesDebugInputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper가 조향 Yaw 덮어쓰기 (bHelperOverridesSteeringYaw)", ToolTip="True이면 helper 결과로 SteeringYaw를 덮어씁니다."))
	bool bHelperOverridesSteeringYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper가 서스펜션 Z 덮어쓰기 (bHelperOverridesSuspensionZ)", ToolTip="True이면 helper 결과로 SuspensionZ를 덮어씁니다."))
	bool bHelperOverridesSuspensionZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper가 스핀 Pitch 덮어쓰기 (bHelperOverridesSpinPitch)", ToolTip="True이면 helper 결과로 SpinPitch를 덮어씁니다."))
	bool bHelperOverridesSpinPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 모드 사용 (bEnableHelperCompareMode)", ToolTip="True이면 helper와 디버그 파이프 입력 차이를 비교합니다."))
	bool bEnableHelperCompareMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 상세 로그 (bLogHelperCompareDetails)", ToolTip="True이면 helper 비교 상세 로그를 출력합니다."))
	bool bLogHelperCompareDetails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 경고 각도 임계값 (HelperCompareWarnDeltaDeg)", ToolTip="helper 비교 경고 각도 임계값입니다."))
	float HelperCompareWarnDeltaDeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 경고 Z 임계값 (HelperCompareWarnDeltaZ)", ToolTip="helper 비교 경고 Z 임계값입니다."))
	float HelperCompareWarnDeltaZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 폴백 로그 1회 출력 (bLogHelperFallbackOnce)", ToolTip="True이면 helper 미사용/실패 로그를 1회만 출력합니다."))
	bool bLogHelperFallbackOnce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="조향 Yaw 디버그 파이프 사용 (bUseSteeringYawDebugPipe)", ToolTip="True이면 조향 Yaw 값을 디버그 파이프 기준으로 생성합니다."))
	bool bUseSteeringYawDebugPipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="조향 전륜 개수 (FrontWheelCountForSteering)", ToolTip="조향 대상으로 간주할 전륜 휠 개수입니다."))
	int32 FrontWheelCountForSteering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="조향 Yaw 스케일 각도 (SteeringYawScaleDeg)", ToolTip="입력값 1.0 기준으로 적용할 조향 Yaw 스케일 각도(deg)입니다."))
	float SteeringYawScaleDeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="조향 시각 부호 (SteeringVisualSign)", ToolTip="조향 시각 방향을 보정할 부호 값입니다. 일반적으로 1 또는 -1을 사용합니다."))
	float SteeringVisualSign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="서스펜션 Z 디버그 파이프 사용 (bUseSuspensionZDebugPipe)", ToolTip="True이면 서스펜션 Z 값을 디버그 파이프 기준으로 생성합니다."))
	bool bUseSuspensionZDebugPipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="서스펜션 Z 디버그 오프셋 (SuspensionZDebugOffset)", ToolTip="디버그 파이프 기준 서스펜션 Z에 추가할 오프셋입니다."))
	float SuspensionZDebugOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="서스펜션 시각 부호 (SuspensionVisualSign)", ToolTip="서스펜션 시각 방향을 보정할 부호 값입니다. 일반적으로 1 또는 -1을 사용합니다."))
	float SuspensionVisualSign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 스핀 Pitch 디버그 파이프 사용 (bUseWheelSpinPitchDebugPipe)", ToolTip="True이면 휠 스핀 Pitch 값을 디버그 파이프 기준으로 생성합니다."))
	bool bUseWheelSpinPitchDebugPipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 스핀 Pitch 디버그 각도 (WheelSpinPitchDebugDeg)", ToolTip="디버그 파이프 기준으로 사용할 휠 스핀 Pitch 각도(deg)입니다."))
	float WheelSpinPitchDebugDeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 스핀 시각 부호 (WheelSpinVisualSign)", ToolTip="휠 스핀 시각 방향을 보정할 부호 값입니다. 일반적으로 1 또는 -1을 사용합니다."))
	float WheelSpinVisualSign;

	// Wheel mesh의 로컬 Pitch 축 방향 보정용 부호입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 메쉬 Pitch 축 부호 (WheelSpinMeshAxisSign)", ToolTip="Wheel mesh의 로컬 Pitch 축 방향 보정용 부호입니다. 현재 프로젝트 PoliceCar 기준 기본값은 -1입니다."))
	float WheelSpinMeshAxisSign;

	// 매우 작은 휠 각속도를 0으로 보는 dead zone 임계값(deg/s)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 각속도 데드존 (WheelSpinAngularVelocityDeadZoneDegPerSec)", ToolTip="매우 작은 휠 각속도를 0으로 보는 dead zone 임계값(deg/s)입니다."))
	float WheelSpinAngularVelocityDeadZoneDegPerSec;

	// Forward 속도를 방향 안정화 기준으로 사용하기 위한 dead zone 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="전진 속도 데드존 (WheelSpinForwardDeadZoneKmh)", ToolTip="Forward 속도를 방향 안정화 기준으로 사용하기 위한 dead zone 임계값(km/h)입니다."))
	float WheelSpinForwardDeadZoneKmh;

	// 휠 자체 각속도 부호를 신뢰할 만큼 충분히 강한 슬립으로 보는 임계값(deg/s)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 슬립 각속도 임계값 (WheelSpinSlipAngularVelocityThresholdDegPerSec)", ToolTip="휠 자체 각속도 부호를 신뢰할 만큼 충분히 강한 슬립으로 보는 임계값(deg/s)입니다."))
	float WheelSpinSlipAngularVelocityThresholdDegPerSec;

	// 방향 반전 후보가 이 시간 이상 유지될 때만 실제 stable sign을 바꾸는 hold 시간(sec)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="방향 전환 유지 시간 (WheelSpinDirectionChangeHoldTimeSeconds)", ToolTip="방향 반전 후보가 이 시간 이상 유지될 때만 실제 stable sign을 바꾸는 hold 시간(sec)입니다."))
	float WheelSpinDirectionChangeHoldTimeSeconds;

	// 접지 중 저속 상태에서 visual spin 과회전을 막기 위해 상한을 적용할 최대 Forward 속도(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="저속 시각 스핀 상한 속도 (WheelSpinLowSpeedVisualCapMaxKmh)", ToolTip="접지 중 저속 상태에서 visual spin 과회전을 막기 위해 상한을 적용할 최대 Forward 속도(km/h)입니다."))
	float WheelSpinLowSpeedVisualCapMaxKmh;

	// 접지 중 저속 상태에서 무슬립 기준 각속도에 허용할 최대 overspeed 배수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="저속 최대 오버스피드 배수 (WheelSpinLowSpeedMaxOverspeedRatio)", ToolTip="접지 중 저속 상태에서 무슬립 기준 각속도에 허용할 최대 overspeed 배수입니다."))
	float WheelSpinLowSpeedMaxOverspeedRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 Anchor 컴포넌트 이름 목록 (WheelAnchorComponentNames)", ToolTip="휠 Anchor로 사용할 Scene Component 이름 목록입니다. ExpectedWheelCount 순서와 맞춰 관리합니다."))
	TArray<FName> WheelAnchorComponentNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="휠 메쉬 컴포넌트 이름 목록 (WheelMeshComponentNames)", ToolTip="휠 Mesh로 사용할 Static Mesh Component 이름 목록입니다. ExpectedWheelCount 순서와 맞춰 관리합니다."))
	TArray<FName> WheelMeshComponentNames;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="캐시된 Vehicle Movement 컴포넌트 (CachedVehicleMovementComponent)", ToolTip="Owner에서 찾은 Chaos Wheeled Vehicle Movement Component 캐시입니다."))
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedVehicleMovementComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="캐시된 휠 Anchor 컴포넌트 목록 (CachedWheelAnchorComponents)", ToolTip="이름 탐색 또는 수동 준비 이후 캐시한 휠 Anchor 컴포넌트 배열입니다."))
	TArray<TObjectPtr<USceneComponent>> CachedWheelAnchorComponents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="캐시된 휠 메쉬 컴포넌트 목록 (CachedWheelMeshComponents)", ToolTip="이름 탐색 또는 수동 준비 이후 캐시한 휠 메쉬 컴포넌트 배열입니다."))
	TArray<TObjectPtr<UStaticMeshComponent>> CachedWheelMeshComponents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="기준 Anchor 위치 목록 (BaseWheelAnchorLocations)", ToolTip="초기 캡처 시점의 휠 Anchor 기준 위치 배열입니다."))
	TArray<FVector> BaseWheelAnchorLocations;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="기준 Anchor 회전 목록 (BaseWheelAnchorRotations)", ToolTip="초기 캡처 시점의 휠 Anchor 기준 회전 배열입니다."))
	TArray<FRotator> BaseWheelAnchorRotations;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="기준 휠 메쉬 회전 목록 (BaseWheelMeshRotations)", ToolTip="초기 캡처 시점의 휠 메쉬 기준 회전 배열입니다."))
	TArray<FRotator> BaseWheelMeshRotations;

	// 마지막 프레임에 읽은 실제 휠 회전각(deg) 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="마지막 실제 휠 각도 목록 (LastRuntimeWheelAnglesDeg)", ToolTip="마지막 프레임에 읽은 실제 휠 회전각(deg) 배열입니다."))
	TArray<float> LastRuntimeWheelAnglesDeg;

	// 래핑을 풀어서 누적한 실제 휠 시각 스핀 각(deg) 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="누적 실제 휠 스핀 목록 (AccumulatedRuntimeWheelSpinDeg)", ToolTip="래핑을 풀어서 누적한 실제 휠 시각 스핀 각(deg) 배열입니다."))
	TArray<float> AccumulatedRuntimeWheelSpinDeg;

	// 실제 휠 회전각의 이전 프레임 샘플이 유효한지 나타내는 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="휠 각도 샘플 유효 여부 목록 (bHasRuntimeWheelAngleSample)", ToolTip="실제 휠 회전각의 이전 프레임 샘플이 유효한지 나타내는 배열입니다."))
	TArray<bool> bHasRuntimeWheelAngleSample;

	// 현재 휠별로 시각 회전에 사용 중인 안정화 방향 부호 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="안정화 휠 방향 부호 목록 (StableWheelSpinDirectionSigns)", ToolTip="현재 휠별로 시각 회전에 사용 중인 안정화 방향 부호 배열입니다."))
	TArray<int32> StableWheelSpinDirectionSigns;

	// 현재 휠별로 방향 전환 후보 부호를 임시 보관하는 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="대기 중 휠 방향 부호 목록 (PendingWheelSpinDirectionSigns)", ToolTip="현재 휠별로 방향 전환 후보 부호를 임시 보관하는 배열입니다."))
	TArray<int32> PendingWheelSpinDirectionSigns;

	// 현재 휠별로 방향 전환 후보가 유지된 누적 시간(sec) 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="방향 전환 유지 시간 목록 (WheelSpinDirectionHoldElapsedSeconds)", ToolTip="현재 휠별로 방향 전환 후보가 유지된 누적 시간(sec) 배열입니다."))
	TArray<float> WheelSpinDirectionHoldElapsedSeconds;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="마지막 Wheel Visual 입력 목록 (LastWheelVisualInputs)", ToolTip="마지막으로 계산된 Wheel Visual 입력 배열입니다."))
	TArray<FCFWheelVisualInput> LastWheelVisualInputs;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="마지막 Wheel Visual 상태 목록 (LastWheelVisualStates)", ToolTip="마지막으로 적용된 Wheel Visual 상태 배열입니다."))
	TArray<FCFWheelVisualState> LastWheelVisualStates;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 상태 목록 (LastHelperCompareStates)", ToolTip="helper 입력 vs 디버그 파이프 입력 비교 상태 배열입니다."))
	TArray<FCFWheelHelperCompareState> LastHelperCompareStates;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 요약 (LastHelperCompareSummary)", ToolTip="helper 비교 요약 문자열입니다."))
	FString LastHelperCompareSummary;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 경고 휠 목록 (LastHelperCompareWarnWheelIndices)", ToolTip="helper 비교 경고 휠 인덱스 목록 문자열입니다."))
	FString LastHelperCompareWarnWheelIndices;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, DisplayName="Helper 비교 전후륜 요약 (LastHelperCompareFrontRearSummary)", ToolTip="helper 비교 전/후륜 요약 문자열입니다."))
	FString LastHelperCompareFrontRearSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Legacy", meta=(EditCondition="bDebugMode", EditConditionHides, DeprecatedProperty, DeprecationMessage="Legacy single-axis test field. No longer used by CFWheelSyncComp runtime.", DisplayName="레거시 단일축 테스트 사용 (bEnableSingleAxisSteeringYawCppApplyOnly)", ToolTip="레거시 단일축 테스트 필드입니다. 현재 런타임에서는 사용하지 않습니다."))
	bool bEnableSingleAxisSteeringYawCppApplyOnly;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Legacy", meta=(EditCondition="bDebugMode", EditConditionHides, DeprecatedProperty, DeprecationMessage="Legacy single-axis summary field. No longer updated by CFWheelSyncComp runtime.", DisplayName="레거시 단일축 테스트 요약 (SingleAxisCppApplyTestSummary)", ToolTip="레거시 단일축 테스트 요약 문자열입니다. 현재 런타임에서는 갱신하지 않습니다."))
	FString SingleAxisCppApplyTestSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="조향 Yaw C++ 적용 (bApplySteeringYawInCpp)", ToolTip="True이면 최종 조향 Yaw 적용을 C++에서 수행합니다."))
	bool bApplySteeringYawInCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="서스펜션 Z C++ 적용 (bApplySuspensionZInCpp)", ToolTip="True이면 최종 서스펜션 Z 적용을 C++에서 수행합니다."))
	bool bApplySuspensionZInCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(DisplayName="스핀 Pitch C++ 적용 (bApplySpinPitchInCpp)", ToolTip="True이면 최종 스핀 Pitch 적용을 C++에서 수행합니다."))
	bool bApplySpinPitchInCpp;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="WheelSync 준비 완료 여부 (bWheelSyncReady)", ToolTip="현재 WheelSync 준비와 검증이 완료되었는지 여부입니다."))
	bool bWheelSyncReady;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="마지막 검증 요약 (LastValidationSummary)", ToolTip="마지막 WheelSync 검증 결과 요약 문자열입니다."))
	FString LastValidationSummary;

	// 마지막 Phase2 입력 생성 요약 문자열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync", meta=(DisplayName="마지막 입력 생성 요약 (LastInputBuildSummary)", ToolTip="마지막 Phase2 입력 생성 요약 문자열입니다."))
	FString LastInputBuildSummary;

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	void ResetWheelSyncState();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool CacheVehicleMovementComponent();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool BuildWheelComponentCache();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool CaptureBaseWheelVisualState();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool ValidateWheelSyncSetup();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool TryPrepareWheelSync();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool BuildWheelVisualInputsFromDebugPipe(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs);

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool BuildWheelVisualInputsPhase2(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs);

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool ApplyWheelVisualInputsPhase2(const TArray<FCFWheelVisualInput>& InWheelInputs);

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool UpdateWheelVisualsPhase2(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsWheelSyncReady() const;

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsSteeringYawOwnedByCpp() const;

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsSuspensionZOwnedByCpp() const;

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsSpinPitchOwnedByCpp() const;

private:
	void InitializeDefaultWheelNames();
	USceneComponent* FindSceneComponentByName(FName ComponentName) const;
	UStaticMeshComponent* FindStaticMeshComponentByName(FName ComponentName) const;
	bool ApplySingleWheelInputPhase2(const FCFWheelVisualInput& WheelInput);
	bool TryApplyRealTransformHelperToInput(int32 WheelIndex, const FCFWheelVisualInput& DebugWheelInput, FCFWheelVisualInput& InOutFinalWheelInput);
	void ResetHelperCompareStates();
	void FinalizeHelperCompareSummary();
	bool IsFrontWheelIndexForSteering(int32 WheelIndex) const;
	void ResetBaseVisualCaches();
	void ResetLastWheelStates();

	// 실제 휠 회전각 샘플 캐시를 초기화합니다.
	void ResetRuntimeWheelSpinCaches();

	// 현재 차량 Forward 속도(km/h)를 WheelSync 기준으로 계산합니다.
	float GetCurrentForwardSpeedKmhForWheelSpin() const;

	// 휠 각속도와 Forward 속도 기준으로 원하는 회전 방향 부호를 결정합니다.
	int32 ResolveDesiredWheelSpinDirectionSign(float InForwardSpeedKmh, float InWheelAngularVelocityDegPerSec) const;

	// 휠별 방향 hold 상태를 반영해 실제 stable sign을 갱신합니다.
	int32 UpdateStableWheelSpinDirectionSign(int32 InWheelIndex, int32 InDesiredDirectionSign, float InDeltaSeconds);
	bool bHasLoggedHelperFallback;
	bool FailValidation(const FString& FailureMessage);
	bool PassValidation(const FString& SuccessMessage);
};
