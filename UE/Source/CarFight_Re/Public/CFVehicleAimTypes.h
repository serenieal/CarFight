// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-07-02
// Description: CarFight 싱글플레이 차량 Aim 시스템의 공용 타입 정의
// Changelog:
// - v1.5.0: 터렛 안정화 전 발사 정책에 맞춰 조준각 내부 여부를 발사 차단 조건이 아닌 표시/디버그 상태로 정리.
// - v1.4.0: 싱글플레이 기준에 맞춰 NetSerialization 의존성과 FVector_NetQuantize 계열 타입을 제거하고 일반 FVector로 통일.
// - v1.3.0: Reticle/FireReject enum 값에서 서버 중심 이름을 싱글플레이 로컬 발사 처리 이름으로 교체.
// - v1.2.0: 발사 검증/시각 상태 타입과 필드명을 싱글플레이 용어로 리네이밍.
// - v1.1.0: 싱글플레이 전환에 맞춰 표시명과 툴팁을 로컬 Fire Command / Fire Result 의미로 정리.
// Migration:
// - OutOfWeaponArc는 호환을 위해 유지하지만, P0 터렛 발사 정책에서는 기본 발사 거부 사유로 사용하지 않는다.
// - ECFVehicleReticleState::WaitingServer는 FirePending으로 교체한다.
// - ECFVehicleReticleState::ServerRejected는 FireRejected로 교체한다.
// - ECFVehicleFireRejectReason::NoAuthority는 InvalidLocalState로 교체한다.
// - ECFVehicleFireRejectReason::ServerTraceMiss는 TraceMiss로 교체한다.
// - FVector_NetQuantize / FVector_NetQuantizeNormal 필드는 FVector로 교체한다.
// - FCFVehicleServerAimState는 FCFVehicleFireValidationState로 교체한다.
// - FCFVehicleRepAimVisualState는 FCFVehicleAimVisualState로 교체한다.
// - Server/Rep 접두 필드는 Validation/Visual/Local 접두 필드로 교체한다.
// Scope: Reticle 상태, 발사 거부 사유, Aim Profile, Local Aim 상태, 로컬 발사 검증/시각 상태, Fire Command/Result 구조를 제공합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleAimTypes.generated.h"

/**
 * 차량 조준점 UI가 표시할 수 있는 대표 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleReticleState : uint8
{
	Hidden UMETA(DisplayName="Hidden"),
	Ready UMETA(DisplayName="Ready"),
	Blocked UMETA(DisplayName="Blocked"),
	OutOfArc UMETA(DisplayName="OutOfArc"),
	NoWeapon UMETA(DisplayName="NoWeapon"),
	Cooldown UMETA(DisplayName="Cooldown"),
	Reloading UMETA(DisplayName="Reloading"),
	FirePending UMETA(DisplayName="FirePending"),
	FireRejected UMETA(DisplayName="FireRejected")
};

/**
 * 로컬 발사 명령을 거부할 때 사용할 대표 사유입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleFireRejectReason : uint8
{
	None UMETA(DisplayName="None"),
	InvalidLocalState UMETA(DisplayName="InvalidLocalState"),
	InvalidOwner UMETA(DisplayName="InvalidOwner"),
	VehicleDisabled UMETA(DisplayName="VehicleDisabled"),
	NoWeapon UMETA(DisplayName="NoWeapon"),
	WeaponCooldown UMETA(DisplayName="WeaponCooldown"),
	NoAmmo UMETA(DisplayName="NoAmmo"),
	OutOfWeaponArc UMETA(DisplayName="OutOfWeaponArc"),
	AimBlocked UMETA(DisplayName="AimBlocked"),
	InvalidAimOrigin UMETA(DisplayName="InvalidAimOrigin"),
	InvalidAimDirection UMETA(DisplayName="InvalidAimDirection"),
	TraceMiss UMETA(DisplayName="TraceMiss")
};

/**
 * 차량 무기 또는 조준 그룹이 사용할 기본 Aim 제한 프로필입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleAimProfile
{
	GENERATED_BODY()

	// [v1.0.0] 현재 Aim Profile을 식별하는 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="프로필 이름 (ProfileName)", ToolTip="현재 Aim Profile을 식별하기 위한 이름입니다."))
	FName ProfileName = TEXT("DefaultAimProfile");

	// [v1.0.0] 차량 정면 기준 최소 좌우 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="최소 Yaw 각도 (MinYawDeg)", ToolTip="차량 정면 기준 최소 좌우 조준각입니다. 음수는 왼쪽을 뜻합니다."))
	float MinYawDeg = -20.0f;

	// [v1.0.0] 차량 정면 기준 최대 좌우 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="최대 Yaw 각도 (MaxYawDeg)", ToolTip="차량 정면 기준 최대 좌우 조준각입니다. 양수는 오른쪽을 뜻합니다."))
	float MaxYawDeg = 20.0f;

	// [v1.0.0] 차량 기준 최소 상하 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="최소 Pitch 각도 (MinPitchDeg)", ToolTip="차량 기준 최소 상하 조준각입니다. 음수는 아래를 뜻합니다."))
	float MinPitchDeg = -10.0f;

	// [v1.0.0] 차량 기준 최대 상하 조준각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="최대 Pitch 각도 (MaxPitchDeg)", ToolTip="차량 기준 최대 상하 조준각입니다. 양수는 위를 뜻합니다."))
	float MaxPitchDeg = 10.0f;

	// [v1.0.0] 이 프로필이 허용하는 최대 조준 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(ClampMin="0.0", DisplayName="최대 조준 거리 (MaxAimDistance)", ToolTip="이 Aim Profile이 허용하는 최대 조준 거리입니다."))
	float MaxAimDistance = 10000.0f;
};

/**
 * 로컬 플레이어가 즉시 표시할 조준 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleLocalAimState
{
	GENERATED_BODY()

	// [v1.4.0] 로컬 기준 현재 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 조준 목표 위치 (LocalAimTargetLocation)", ToolTip="로컬 플레이어 기준 현재 조준 목표 월드 위치입니다."))
	FVector LocalAimTargetLocation = FVector::ZeroVector;

	// [v1.4.0] 로컬 기준 현재 조준 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 조준 방향 (LocalAimDirection)", ToolTip="로컬 플레이어 기준 현재 조준 방향입니다."))
	FVector LocalAimDirection = FVector::ForwardVector;

	// [v1.1.0] 로컬 기준 현재 Reticle 표시 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 Reticle 상태 (LocalReticleState)", ToolTip="로컬 플레이어 기준 현재 조준점 표시 상태입니다."))
	ECFVehicleReticleState LocalReticleState = ECFVehicleReticleState::Hidden;

	// [v1.5.0] 로컬 예측 기준 발사 가능 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 발사 가능 예측 (bLocalCanFire)", ToolTip="로컬 예측 기준 현재 발사 가능 여부입니다. 조준각 초과는 단독 발사 차단 조건이 아닙니다."))
	bool bLocalCanFire = false;

	// [v1.5.0] 로컬 기준 무기 조준각 안에 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 무기 조준각 내부 여부 (bLocalWithinWeaponArc)", ToolTip="로컬 기준 현재 조준이 무기 조준각 안에 들어오는지 여부입니다. 표시/디버그용 상태이며 단독 발사 차단 조건이 아닙니다."))
	bool bLocalWithinWeaponArc = false;

	// [v1.0.0] 로컬 기준 조준이 장애물 등에 막힌 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 조준 가림 여부 (bLocalAimBlocked)", ToolTip="로컬 기준 현재 조준이 장애물 등에 막힌 상태인지 여부입니다."))
	bool bLocalAimBlocked = false;
};

/**
 * 로컬 발사 검증에 사용할 조준 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFireValidationState
{
	GENERATED_BODY()

	// [v1.4.0] 로컬 검증 기준 현재 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="검증 조준 목표 위치 (ValidationAimTargetLocation)", ToolTip="로컬 발사 검증 기준 현재 조준 목표 월드 위치입니다."))
	FVector ValidationAimTargetLocation = FVector::ZeroVector;

	// [v1.5.0] 로컬 검증 기준 무기 조준각 안에 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="검증 무기 조준각 내부 여부 (bValidationWithinWeaponArc)", ToolTip="로컬 발사 검증 기준 현재 조준이 무기 조준각 안에 들어오는지 여부입니다. 표시/디버그용 상태이며 단독 발사 차단 조건이 아닙니다."))
	bool bValidationWithinWeaponArc = false;

	// [v1.2.0] 로컬 검증 기준 발사 가능 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="검증 발사 가능 여부 (bValidationCanFire)", ToolTip="로컬 발사 검증 기준 현재 발사 가능 여부입니다."))
	bool bValidationCanFire = false;

	// [v1.2.0] 마지막으로 기록한 로컬 발사 거부 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="마지막 발사 거부 사유 (LastValidationRejectReason)", ToolTip="로컬 발사 검증이 마지막으로 기록한 발사 명령 거부 사유입니다."))
	ECFVehicleFireRejectReason LastValidationRejectReason = ECFVehicleFireRejectReason::None;

	// [v1.1.0] 마지막으로 승인한 발사 명령 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="마지막 승인 발사 명령 ID (LastAcceptedFireRequestId)", ToolTip="로컬 발사 검증이 마지막으로 승인한 발사 명령 ID입니다."))
	int32 LastAcceptedFireRequestId = 0;

	// [v1.1.0] 마지막으로 거부한 발사 명령 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="마지막 거부 발사 명령 ID (LastRejectedFireRequestId)", ToolTip="로컬 발사 검증이 마지막으로 거부한 발사 명령 ID입니다."))
	int32 LastRejectedFireRequestId = 0;
};

/**
 * 로컬 디버그와 발사 피드백에 사용할 최소 조준 시각 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleAimVisualState
{
	GENERATED_BODY()

	// [v1.4.0] 표시 시각화용 조준 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="표시 조준 방향 (VisualAimDirection)", ToolTip="로컬 디버그와 발사 피드백에 표시할 조준 방향입니다."))
	FVector VisualAimDirection = FVector::ForwardVector;

	// [v1.4.0] 표시 시각화용 조준 목표 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="표시 조준 목표 위치 (VisualAimTargetLocation)", ToolTip="로컬 디버그와 발사 피드백에 표시할 조준 목표 월드 위치입니다."))
	FVector VisualAimTargetLocation = FVector::ZeroVector;

	// [v1.1.0] 발사 이펙트 시각화 재생 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="발사 시각화 여부 (bIsFiringVisual)", ToolTip="로컬 발사 피드백에서 발사 이펙트 시각화를 표시할지 여부입니다."))
	bool bIsFiringVisual = false;

	// [v1.2.0] 무기 시각화 모드를 구분하는 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="무기 시각화 모드 (WeaponVisualMode)", ToolTip="로컬 발사 피드백에 사용할 무기 시각화 모드 이름입니다."))
	FName WeaponVisualMode = NAME_None;
};

/**
 * 로컬 발사 검증에 넘기는 발사 명령 입력 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFireRequest
{
	GENERATED_BODY()

	// [v1.1.0] 로컬 발사 명령 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="발사 명령 ID (FireRequestId)", ToolTip="로컬 플레이어가 생성한 발사 명령 ID입니다. 필드명은 기존 BP 호환을 위해 유지합니다."))
	int32 FireRequestId = 0;

	// [v1.1.0] 로컬 플레이어가 발사를 입력한 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="로컬 발사 시간 (ClientFireTimeSeconds)", ToolTip="로컬 플레이어가 발사를 입력한 월드 시간(초)입니다. 필드명은 기존 BP 호환을 위해 유지합니다."))
	float ClientFireTimeSeconds = 0.0f;

	// [v1.4.0] 로컬 발사 검증용 조준 시작 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="조준 시작 위치 (AimOrigin)", ToolTip="로컬 발사 검증용 조준 시작 월드 위치입니다."))
	FVector AimOrigin = FVector::ZeroVector;

	// [v1.4.0] 로컬 발사 검증용 조준 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="조준 방향 (AimDirection)", ToolTip="로컬 발사 검증용 조준 방향입니다."))
	FVector AimDirection = FVector::ForwardVector;

	// [v1.4.0] 로컬 발사 명령이 참조하는 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="조준 목표 위치 (PredictedAimTargetLocation)", ToolTip="로컬 발사 명령이 참조하는 조준 목표 월드 위치입니다. 필드명은 기존 BP 호환을 위해 유지합니다."))
	FVector PredictedAimTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 발사를 요청한 무기 그룹 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="무기 그룹 ID (WeaponGroupId)", ToolTip="발사를 요청한 무기 그룹 ID입니다."))
	FName WeaponGroupId = NAME_None;
};

/**
 * 로컬 발사 명령 처리 후 적용할 결과 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFireResult
{
	GENERATED_BODY()

	// [v1.1.0] 처리된 발사 명령 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="발사 명령 ID (FireRequestId)", ToolTip="로컬 검증에서 처리한 발사 명령 ID입니다. 필드명은 기존 BP 호환을 위해 유지합니다."))
	int32 FireRequestId = 0;

	// [v1.1.0] 로컬 검증이 발사 명령을 승인했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="발사 승인 여부 (bAccepted)", ToolTip="로컬 검증이 발사 명령을 승인했는지 여부입니다."))
	bool bAccepted = false;

	// [v1.1.0] 로컬 검증이 발사 명령을 거부한 경우의 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="거부 사유 (RejectReason)", ToolTip="로컬 검증이 발사 명령을 거부한 경우의 사유입니다. 승인된 경우 None입니다."))
	ECFVehicleFireRejectReason RejectReason = ECFVehicleFireRejectReason::None;

	// [v1.4.0] 로컬 검증 기준 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="검증 조준 목표 위치 (ValidationAimTargetLocation)", ToolTip="로컬 검증 기준으로 확정한 조준 목표 월드 위치입니다."))
	FVector ValidationAimTargetLocation = FVector::ZeroVector;

	// [v1.4.0] 로컬 Trace 기준 적중 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="로컬 적중 위치 (LocalHitLocation)", ToolTip="로컬 Trace 기준 적중 위치입니다."))
	FVector LocalHitLocation = FVector::ZeroVector;

	// [v1.4.0] 로컬 Trace 기준 적중 노멀입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="로컬 적중 노멀 (LocalHitNormal)", ToolTip="로컬 Trace 기준 적중 노멀입니다."))
	FVector LocalHitNormal = FVector::UpVector;
};
