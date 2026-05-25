// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-05-21
// Description: CarFight 차량 Aim 시스템의 1차 공용 타입 정의
// Scope: Reticle 상태, 발사 거부 사유, Aim Profile, Local/Server/Replicated Aim 상태, Fire Request/Result 구조를 제공합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
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
	WaitingServer UMETA(DisplayName="WaitingServer"),
	ServerRejected UMETA(DisplayName="ServerRejected")
};

/**
 * 서버가 차량 발사 요청을 거부할 때 사용할 대표 사유입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleFireRejectReason : uint8
{
	None UMETA(DisplayName="None"),
	NoAuthority UMETA(DisplayName="NoAuthority"),
	InvalidOwner UMETA(DisplayName="InvalidOwner"),
	VehicleDisabled UMETA(DisplayName="VehicleDisabled"),
	NoWeapon UMETA(DisplayName="NoWeapon"),
	WeaponCooldown UMETA(DisplayName="WeaponCooldown"),
	NoAmmo UMETA(DisplayName="NoAmmo"),
	OutOfWeaponArc UMETA(DisplayName="OutOfWeaponArc"),
	AimBlocked UMETA(DisplayName="AimBlocked"),
	InvalidAimOrigin UMETA(DisplayName="InvalidAimOrigin"),
	InvalidAimDirection UMETA(DisplayName="InvalidAimDirection"),
	ServerTraceMiss UMETA(DisplayName="ServerTraceMiss")
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
 * 소유 클라이언트가 즉시 표시할 로컬 조준 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleLocalAimState
{
	GENERATED_BODY()

	// [v1.0.0] 로컬 기준 현재 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 조준 목표 위치 (LocalAimTargetLocation)", ToolTip="로컬 클라이언트 기준 현재 조준 목표 월드 위치입니다."))
	FVector_NetQuantize LocalAimTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 로컬 기준 현재 조준 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 조준 방향 (LocalAimDirection)", ToolTip="로컬 클라이언트 기준 현재 조준 방향입니다."))
	FVector_NetQuantizeNormal LocalAimDirection = FVector::ForwardVector;

	// [v1.0.0] 로컬 기준 현재 Reticle 표시 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 Reticle 상태 (LocalReticleState)", ToolTip="로컬 클라이언트 기준 현재 조준점 표시 상태입니다."))
	ECFVehicleReticleState LocalReticleState = ECFVehicleReticleState::Hidden;

	// [v1.0.0] 로컬 예측 기준 발사 가능 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 발사 가능 예측 (bLocalCanFire)", ToolTip="로컬 예측 기준 현재 발사 가능 여부입니다. 서버 최종 판정은 아닙니다."))
	bool bLocalCanFire = false;

	// [v1.0.0] 로컬 기준 무기 조준각 안에 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 무기 조준각 내부 여부 (bLocalWithinWeaponArc)", ToolTip="로컬 기준 현재 조준이 무기 조준각 안에 들어오는지 여부입니다."))
	bool bLocalWithinWeaponArc = false;

	// [v1.0.0] 로컬 기준 조준이 장애물 등에 막힌 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 조준 가림 여부 (bLocalAimBlocked)", ToolTip="로컬 기준 현재 조준이 장애물 등에 막힌 상태인지 여부입니다."))
	bool bLocalAimBlocked = false;
};

/**
 * 서버가 발사 요청 검증에 사용할 조준 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleServerAimState
{
	GENERATED_BODY()

	// [v1.0.0] 서버 기준 현재 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="서버 조준 목표 위치 (ServerAimTargetLocation)", ToolTip="서버 검증 기준 현재 조준 목표 월드 위치입니다."))
	FVector_NetQuantize ServerAimTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 서버 기준 무기 조준각 안에 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="서버 무기 조준각 내부 여부 (bServerWithinWeaponArc)", ToolTip="서버 검증 기준 현재 조준이 무기 조준각 안에 들어오는지 여부입니다."))
	bool bServerWithinWeaponArc = false;

	// [v1.0.0] 서버 최종 판정 기준 발사 가능 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="서버 발사 가능 여부 (bServerCanFire)", ToolTip="서버 최종 판정 기준 현재 발사 가능 여부입니다."))
	bool bServerCanFire = false;

	// [v1.0.0] 서버가 마지막으로 기록한 발사 거부 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="마지막 서버 거부 사유 (LastServerRejectReason)", ToolTip="서버가 마지막으로 기록한 발사 요청 거부 사유입니다."))
	ECFVehicleFireRejectReason LastServerRejectReason = ECFVehicleFireRejectReason::None;

	// [v1.0.0] 서버가 마지막으로 승인한 발사 요청 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="마지막 승인 발사 요청 ID (LastAcceptedFireRequestId)", ToolTip="서버가 마지막으로 승인한 발사 요청 ID입니다."))
	int32 LastAcceptedFireRequestId = 0;

	// [v1.0.0] 서버가 마지막으로 거부한 발사 요청 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="마지막 거부 발사 요청 ID (LastRejectedFireRequestId)", ToolTip="서버가 마지막으로 거부한 발사 요청 ID입니다."))
	int32 LastRejectedFireRequestId = 0;
};

/**
 * 다른 클라이언트에게 보여줄 최소 조준 시각 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleRepAimVisualState
{
	GENERATED_BODY()

	// [v1.0.0] 복제 시각화용 조준 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="복제 조준 방향 (RepAimDirection)", ToolTip="다른 클라이언트에게 보여줄 조준 방향입니다. 전투 판정용 데이터가 아닙니다."))
	FVector_NetQuantizeNormal RepAimDirection = FVector::ForwardVector;

	// [v1.0.0] 복제 시각화용 조준 목표 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="복제 조준 목표 위치 (RepAimTargetLocation)", ToolTip="다른 클라이언트에게 보여줄 조준 목표 월드 위치입니다. 전투 판정용 데이터가 아닙니다."))
	FVector_NetQuantize RepAimTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 발사 이펙트 시각화 재생 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="발사 시각화 여부 (bIsFiringVisual)", ToolTip="다른 클라이언트에게 발사 이펙트 시각화를 보여줄지 여부입니다."))
	bool bIsFiringVisual = false;

	// [v1.0.0] 무기 시각화 모드를 구분하는 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="무기 시각화 모드 (RepWeaponVisualMode)", ToolTip="다른 클라이언트에게 보여줄 무기 시각화 모드 이름입니다."))
	FName RepWeaponVisualMode = NAME_None;
};

/**
 * 클라이언트가 서버에 보내는 발사 요청 입력 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFireRequest
{
	GENERATED_BODY()

	// [v1.0.0] 클라이언트가 생성한 발사 요청 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="발사 요청 ID (FireRequestId)", ToolTip="클라이언트가 생성한 발사 요청 ID입니다."))
	int32 FireRequestId = 0;

	// [v1.0.0] 클라이언트가 발사를 요청한 로컬 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="클라이언트 발사 시간 (ClientFireTimeSeconds)", ToolTip="클라이언트가 발사를 요청한 로컬 월드 시간(초)입니다."))
	float ClientFireTimeSeconds = 0.0f;

	// [v1.0.0] 클라이언트가 보낸 조준 시작 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="조준 시작 위치 (AimOrigin)", ToolTip="클라이언트가 서버 검증용으로 보낸 조준 시작 월드 위치입니다."))
	FVector_NetQuantize AimOrigin = FVector::ZeroVector;

	// [v1.0.0] 클라이언트가 보낸 조준 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="조준 방향 (AimDirection)", ToolTip="클라이언트가 서버 검증용으로 보낸 조준 방향입니다."))
	FVector_NetQuantizeNormal AimDirection = FVector::ForwardVector;

	// [v1.0.0] 클라이언트가 예측한 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="예측 조준 목표 위치 (PredictedAimTargetLocation)", ToolTip="클라이언트가 예측한 조준 목표 월드 위치입니다. 서버 최종 결과는 아닙니다."))
	FVector_NetQuantize PredictedAimTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 발사를 요청한 무기 그룹 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="무기 그룹 ID (WeaponGroupId)", ToolTip="발사를 요청한 무기 그룹 ID입니다."))
	FName WeaponGroupId = NAME_None;
};

/**
 * 서버가 발사 요청 처리 후 소유 클라이언트에 돌려줄 결과 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFireResult
{
	GENERATED_BODY()

	// [v1.0.0] 처리된 발사 요청 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="발사 요청 ID (FireRequestId)", ToolTip="서버가 처리한 발사 요청 ID입니다."))
	int32 FireRequestId = 0;

	// [v1.0.0] 서버가 발사 요청을 승인했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="발사 승인 여부 (bAccepted)", ToolTip="서버가 발사 요청을 승인했는지 여부입니다."))
	bool bAccepted = false;

	// [v1.0.0] 서버가 발사 요청을 거부한 경우의 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="거부 사유 (RejectReason)", ToolTip="서버가 발사 요청을 거부한 경우의 사유입니다. 승인된 경우 None입니다."))
	ECFVehicleFireRejectReason RejectReason = ECFVehicleFireRejectReason::None;

	// [v1.0.0] 서버 기준 조준 목표 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="서버 조준 목표 위치 (ServerAimTargetLocation)", ToolTip="서버 기준으로 확정한 조준 목표 월드 위치입니다."))
	FVector_NetQuantize ServerAimTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 서버 Trace 기준 적중 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="서버 적중 위치 (ServerHitLocation)", ToolTip="서버 Trace 기준 적중 위치입니다. 이번 1차 골격에서는 실제 Trace를 수행하지 않습니다."))
	FVector_NetQuantize ServerHitLocation = FVector::ZeroVector;

	// [v1.0.0] 서버 Trace 기준 적중 노멀입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(DisplayName="서버 적중 노멀 (ServerHitNormal)", ToolTip="서버 Trace 기준 적중 노멀입니다. 이번 1차 골격에서는 실제 Trace를 수행하지 않습니다."))
	FVector_NetQuantizeNormal ServerHitNormal = FVector::UpVector;
};
