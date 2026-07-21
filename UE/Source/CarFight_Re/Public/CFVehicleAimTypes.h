// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.0
// Date: 2026-07-21
// Description: CarFight 싱글플레이 차량 Aim 시스템의 공용 타입 정의
// Changelog:
// - v1.10.0: 탄종과 착탄 결과에 독립적인 CurrentMuzzleDirection 기반 터렛 레티클 월드 지점을 Weapon Aim Solution에 추가.
// - v1.9.0: Weapon Reticle 표시 모드를 Hidden / DirectImpact / LaunchDirection으로 구분.
// - v1.8.0: Weapon Aim Solution에 직선 사격 Preview 유효성, 첫 Blocking Hit, 월드 위치와 거리를 추가.
// - v1.7.0: Weapon Aim Solution에 정렬 중 발사 정책, 요구 방향, 현재 Muzzle 방향을 추가하고 AimDirection을 실제 최종 발사 방향으로 명확화.
// - v1.6.1: 정렬 중 Reticle 복구를 위해 TurretAligning Reticle 상태를 추가.
// - v1.6.0: Camera Reticle 목표점과 Muzzle 기준 발사 방향/차단 판정을 묶는 Weapon Aim Solution과 신규 거부 사유를 추가.
// - v1.5.0: 터렛 안정화 전 발사 정책에 맞춰 조준각 내부 여부를 발사 차단 조건이 아닌 표시/디버그 상태로 정리.
// - v1.4.0: 싱글플레이 기준에 맞춰 NetSerialization 의존성과 FVector_NetQuantize 계열 타입을 제거하고 일반 FVector로 통일.
// - v1.3.0: Reticle/FireReject enum 값에서 서버 중심 이름을 싱글플레이 로컬 발사 처리 이름으로 교체.
// - v1.2.0: 발사 검증/시각 상태 타입과 필드명을 싱글플레이 용어로 리네이밍.
// - v1.1.0: 싱글플레이 전환에 맞춰 표시명과 툴팁을 로컬 Fire Command / Fire Result 의미로 정리.
// Migration:
// - Image_WeaponReticle은 bHasValidTurretReticlePoint와 TurretReticleWorldLocation을 사용하며 기존 Weapon Preview 필드는 Legacy Debug로만 해석한다.
// - AimDirection은 실제 HitScan/Projectile 최종 방향이며, Reticle 목표 요구 방향은 DesiredAimDirection을 사용한다.
// - WeaponReticleMode가 LaunchDirection이면 Preview 위치는 예상 탄착점이 아니라 실제 초기 발사 방향 표식이다.
// - 정렬 중 발사 정책과 현재 총구 방향은 bAllowFireWhileAligning / CurrentMuzzleDirection에서 확인한다.
// - 터렛/총구 정렬 대기 표시는 ECFVehicleReticleState::TurretAligning을 우선 사용한다.
// - 발사 검증은 LocalAimTargetLocation을 유지하되 FCFVehicleWeaponAimSolution의 AimOrigin/AimDirection/Target을 우선 사용한다.
// - Camera Aim Trace Blocking Hit은 목표 표면 선택 결과이며, MuzzleBlocked만 AimBlocked 피드백으로 처리한다.
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
	TurretAligning UMETA(DisplayName="TurretAligning"),
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
	TraceMiss UMETA(DisplayName="TraceMiss"),
	TurretAligning UMETA(DisplayName="TurretAligning"),
	WeaponNotAligned UMETA(DisplayName="WeaponNotAligned"),
	MuzzleBlocked UMETA(DisplayName="MuzzleBlocked")
};

/**
 * Weapon Reticle 월드 데이터가 어떤 의미로 표시되는지 구분하는 모드입니다.
 */
UENUM(BlueprintType)
enum class ECFWeaponReticleMode : uint8
{
	Hidden UMETA(DisplayName="Hidden"),
	DirectImpact UMETA(DisplayName="DirectImpact"),
	LaunchDirection UMETA(DisplayName="LaunchDirection")
};

/**
 * Camera Reticle 목표점과 실제 무기 발사 기준을 하나로 묶은 조준 해석 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleWeaponAimSolution
{
	GENERATED_BODY()

	// [v1.6.0] 이번 해석 결과가 발사 검증에 사용할 수 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="Weapon Aim Solution 유효 여부 (bHasValidSolution)", ToolTip="Muzzle 위치, Reticle 목표점, 발사 방향이 모두 유효해 발사 검증에 사용할 수 있는지 여부입니다."))
	bool bHasValidSolution = false;

	// [v1.6.0] Camera Aim Trace가 목표 표면 선택용 Blocking Hit을 얻었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="Aim Trace Blocking Hit 여부 (bAimTraceHasBlockingHit)", ToolTip="카메라 Aim Trace가 목표 표면을 선택하기 위해 Blocking Hit을 얻었는지 여부입니다. 이 값만으로 발사를 막지 않습니다."))
	bool bAimTraceHasBlockingHit = false;

	// [v1.6.0] 터렛이 목표 각도 안정화 대기 중인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="터렛 정렬 대기 여부 (bTurretAligning)", ToolTip="터렛이 Reticle 목표점을 향해 이동 중이며 아직 안정화 시간이 끝나지 않았는지 여부입니다."))
	bool bTurretAligning = false;

	// [v1.6.0] 총구 X축과 요구 발사 방향의 정렬 오차가 허용값을 넘었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="무기 정렬 실패 여부 (bWeaponNotAligned)", ToolTip="Muzzle X축과 Reticle 목표점으로 향하는 요구 발사 방향의 각도 차이가 터렛 안정화 허용값을 넘었는지 여부입니다."))
	bool bWeaponNotAligned = false;

	// [v1.7.0] 현재 터렛이 정렬 중 발사를 허용하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="정렬 중 발사 허용 여부 (bAllowFireWhileAligning)", ToolTip="True이면 터렛 또는 총구 정렬 중 실제 최종 발사 방향으로 현재 Muzzle 방향을 사용합니다. False이면 정렬 완료 전 발사 검증에서 거부합니다."))
	bool bAllowFireWhileAligning = false;

	// [v1.7.0] Muzzle에서 실제 최종 발사 방향으로 가는 WeaponHit Trace가 가까운 장애물에 막혔는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="총구 가림 여부 (bMuzzleBlocked)", ToolTip="총구에서 실제 최종 발사 방향으로 수행한 WeaponHit Trace가 검사 거리보다 앞에서 장애물에 막혔는지 여부입니다."))
	bool bMuzzleBlocked = false;

	// [v1.10.0] 현재 터렛 방향을 사용자 조준점과 같은 거리에서 비교할 수 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="터렛 레티클 지점 유효 여부 (bHasValidTurretReticlePoint)", ToolTip="CurrentMuzzleDirection을 사용자 조준점과 같은 거리까지 연장한 터렛 레티클 월드 지점이 유효한지 여부입니다."))
	bool bHasValidTurretReticlePoint = false;

	// [v1.10.0] CurrentMuzzleDirection을 사용자 조준점과 같은 거리까지 연장한 터렛 조준 월드 지점입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="터렛 레티클 월드 위치 (TurretReticleWorldLocation)", ToolTip="Image_WeaponReticle이 화면에 투영할 탄종 독립 터렛 조준 월드 지점입니다. 착탄 위치를 의미하지 않습니다."))
	FVector TurretReticleWorldLocation = FVector::ZeroVector;

	// [v1.10.0] AimOrigin에서 터렛 레티클 월드 지점까지 적용한 비교 거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="터렛 레티클 비교 거리 (TurretReticleDistance)", ToolTip="사용자 조준점과 터렛 조준점을 같은 깊이에서 비교하기 위해 사용한 거리입니다."))
	float TurretReticleDistance = 0.0f;

	// [v1.10.0] Legacy Weapon Preview 월드 데이터의 의미를 나타내는 표시 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="Weapon Reticle 표시 모드 (WeaponReticleMode)", ToolTip="Hidden은 표시 데이터 없음, DirectImpact는 직선 첫 충돌 또는 최대 사거리 끝점, LaunchDirection은 중력 Projectile의 실제 초기 발사 방향 표식입니다."))
	ECFWeaponReticleMode WeaponReticleMode = ECFWeaponReticleMode::Hidden;

	// [v1.9.0] 현재 활성 무기가 Weapon Reticle용 월드 Preview 데이터를 제공할 수 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="무기 Preview 유효 여부 (bHasValidWeaponPreview)", ToolTip="WeaponReticleMode가 Hidden이 아니며 실제 최종 발사 방향의 월드 Preview 위치와 거리가 유효한지 여부입니다."))
	bool bHasValidWeaponPreview = false;

	// [v1.9.0] DirectImpact Preview Trace가 최대 사거리 안에서 첫 Blocking Hit을 얻었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="무기 Preview Blocking Hit 여부 (bWeaponPreviewHasBlockingHit)", ToolTip="DirectImpact 모드에서 실제 최종 발사 방향으로 수행한 WeaponHit Trace가 활성 무기 최대 사거리 안에서 첫 Blocking Hit을 얻었는지 여부입니다. LaunchDirection에서는 항상 false입니다."))
	bool bWeaponPreviewHasBlockingHit = false;

	// [v1.9.0] DirectImpact 충돌 위치 또는 LaunchDirection 방향 표식 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="무기 Preview 월드 위치 (WeaponPreviewWorldLocation)", ToolTip="DirectImpact에서는 첫 Blocking Hit 또는 활성 무기 최대 사거리 끝점입니다. LaunchDirection에서는 중력 Projectile의 초기 발사 방향을 화면에 투영하기 위한 월드 점입니다."))
	FVector WeaponPreviewWorldLocation = FVector::ZeroVector;

	// [v1.8.0] AimOrigin에서 WeaponPreviewWorldLocation까지의 거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="무기 Preview 거리 (WeaponPreviewDistance)", ToolTip="AimOrigin에서 WeaponPreviewWorldLocation까지 측정한 월드 거리입니다."))
	float WeaponPreviewDistance = 0.0f;

	// [v1.6.0] 실제 발사 시작 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="조준 발사 시작 위치 (AimOrigin)", ToolTip="BuildFireCommand, HitScan, Projectile이 공통으로 사용할 실제 발사 시작 월드 위치입니다."))
	FVector AimOrigin = FVector::ZeroVector;

	// [v1.7.0] BuildFireCommand, HitScan, Projectile이 공통으로 사용할 실제 최종 발사 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="최종 발사 방향 (AimDirection)", ToolTip="정렬 완료 시 DesiredAimDirection, 정렬 중 발사 허용 시 CurrentMuzzleDirection을 사용하는 실제 최종 정규화 발사 방향입니다."))
	FVector AimDirection = FVector::ForwardVector;

	// [v1.7.0] Muzzle 위치에서 Reticle 목표점으로 향하는 요구 발사 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="요구 조준 방향 (DesiredAimDirection)", ToolTip="Muzzle 위치에서 Camera Reticle 목표점으로 향하는 정규화된 요구 방향입니다. 터렛 추적과 정렬 오차 계산에 사용합니다."))
	FVector DesiredAimDirection = FVector::ForwardVector;

	// [v1.7.0] 현재 Muzzle Socket X축이 향하는 월드 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="현재 Muzzle 방향 (CurrentMuzzleDirection)", ToolTip="현재 Muzzle Socket X축의 정규화된 월드 방향입니다. 정렬 중 발사가 허용되면 실제 최종 발사 방향으로 사용합니다."))
	FVector CurrentMuzzleDirection = FVector::ForwardVector;

	// [v1.6.0] 실제 발사가 참조하는 Reticle 목표 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="조준 목표 위치 (AimTargetLocation)", ToolTip="Camera Reticle이 선택한 단일 월드 목표 위치입니다."))
	FVector AimTargetLocation = FVector::ZeroVector;

	// [v1.6.0] 총구 X축과 요구 발사 방향 사이의 각도 오차입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Weapon Aim", meta=(DisplayName="무기 정렬 오차 각도 (WeaponAlignmentErrorDeg)", ToolTip="Muzzle X축과 Reticle 목표점 방향 사이의 각도 차이입니다."))
	float WeaponAlignmentErrorDeg = 0.0f;
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

	// [v1.6.0] 로컬 기준 Camera Aim Trace가 목표 표면 선택용 Blocking Hit을 얻었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(DisplayName="로컬 Aim Trace Blocking Hit 여부 (bLocalAimTraceHasBlockingHit)", ToolTip="로컬 Camera Aim Trace가 목표 표면 선택용 Blocking Hit을 얻었는지 여부입니다. 이 값만으로 조준 가림으로 처리하지 않습니다."))
	bool bLocalAimTraceHasBlockingHit = false;
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
