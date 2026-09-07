// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-09-07
// Description: CarFight 물리 제한형 미사일 유도 공용 타입
// Scope: Guidance Mode·Law·Activation, Seeker·관측 성능 모델, 목표 상실 정책, 제한값, 순수 수학 입력·출력과 Debug Snapshot을 제공합니다.
// Changelog:
// - v1.3.0: MG-P0-12D FollowFlightGuidanceWindow/Independent Guidance Activation, 독립 0~180도 Stateful Seeker 반각, activation/overshoot Debug Snapshot을 추가.
// - v1.2.0: MG-P0-12C PurePursuit/LeadPursuit/ProportionalNavigation Guidance Law, bounded lead 설정과 Law Debug Snapshot을 추가.
// - v1.1.0: MG-P0-08 데이터 기반 Guidance Performance를 위한 Legacy/Stateful Seeker, Direct/Sampled 관측, Reacquisition 타입·Config·Snapshot Foundation을 추가.
// - v1.0.0: MG-P0-00 Guidance Config·Command·Snapshot 최초 추가.
// Migration:
// - GuidanceLaw=ProportionalNavigation, GuidanceActivationMode=FollowFlightGuidanceWindow, SeekerModel=LegacySingleGate와 TargetObservationMode=DirectActorKinematics가 기본값이므로 기존 저장 ProjectileData는 재저장 없이 현재 유도 동작을 유지합니다.
// - LegacySingleGate의 기존 SeekerFieldOfViewDeg/LockBreakAngleDeg 의미와 기존 enum 항목 순서는 변경하지 않습니다.
// - Stateful Acquisition/Tracking/Reacquisition 반각은 v1.3.0부터 서로 독립된 0~180도 값이며 Tracking 반각으로 다른 반각을 암묵적으로 축소하지 않습니다.
// - MG-P0-08은 타입·설정 Foundation만 추가하며 Stateful Seeker 상태 전이와 Sampled estimator Runtime은 MG-P0-09~10에서 연결합니다.
// - bUseGuidance=false와 GuideMode=None이 기본값이므로 기존 Projectile·Rocket Velocity를 변경하지 않습니다.
// - 이 파일의 타입은 이동력을 직접 소유하지 않으며 GuideComp가 계산 입력과 결과로 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileGuideTypes.generated.h"

/**
 * 미사일이 관측할 외부 목표 정보의 출처입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileGuideMode : uint8
{
	None UMETA(DisplayName="None"),
	TargetActor UMETA(DisplayName="Target Actor"),
	LaserPoint UMETA(DisplayName="Laser Point"),
	InertialPoint UMETA(DisplayName="Inertial Point"),
	DataLink UMETA(DisplayName="Data Link")
};

/**
 * 미사일 Seeker가 목표를 판정하는 상태 모델입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileSeekerModel : uint8
{
	LegacySingleGate UMETA(DisplayName="기존 단일 각도 판정 (Legacy Single Gate)"),
	Stateful UMETA(DisplayName="상태형 탐색기 (Stateful Seeker)")
};

/**
 * Target Actor의 위치와 속도를 Guidance 입력으로 구성하는 관측 모델입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileTargetObservationMode : uint8
{
	DirectActorKinematics UMETA(DisplayName="Actor 직접 관측 (Direct Actor Kinematics)"),
	SampledPositionEstimate UMETA(DisplayName="위치 샘플 추정 (Sampled Position Estimate)")
};

/**
 * 미사일이 관측된 Target 상태를 실제 조향 명령으로 바꾸는 유도 알고리즘입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileGuidanceLaw : uint8
{
	ProportionalNavigation UMETA(DisplayName="비례항법 (Proportional Navigation)"),
	PurePursuit UMETA(DisplayName="단순 추적 (Pure Pursuit)"),
	LeadPursuit UMETA(DisplayName="제한 선행 추적 (Lead Pursuit)")
};

/**
 * 발사 후 Guidance가 Flight Guidance Window를 따를지 독립 시간·거리 조건으로 열릴지 선택합니다.
 */
UENUM(BlueprintType)
enum class ECFMissileGuidanceActivationMode : uint8
{
	FollowFlightGuidanceWindow UMETA(DisplayName="비행 유도 구간 따름 (Follow Flight Guidance Window)"),
	Independent UMETA(DisplayName="독립 유도 활성화 (Independent Guidance Activation)")
};

/**
 * 상태형 Missile Seeker의 현재 발사 후 추적 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileSeekerState : uint8
{
	Inactive UMETA(DisplayName="비활성 (Inactive)"),
	Acquiring UMETA(DisplayName="최초 획득 중 (Acquiring)"),
	Tracking UMETA(DisplayName="추적 중 (Tracking)"),
	LostGrace UMETA(DisplayName="추적 상실 유예 (Lost Grace)"),
	Reacquiring UMETA(DisplayName="재포착 중 (Reacquiring)"),
	LostFinal UMETA(DisplayName="최종 상실 (Lost Final)")
};

/**
 * 상태형 Seeker가 Launch Target Snapshot을 다시 받아들이는 재포착 정책입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileReacquisitionMode : uint8
{
	None UMETA(DisplayName="재포착 안 함 (None)"),
	ForwardCone UMETA(DisplayName="전방 원뿔 재포착 (Forward Cone)")
};

/**
 * 기존 목표 정보를 더 이상 사용할 수 없을 때의 비행 정책입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileLostTargetPolicy : uint8
{
	ContinueStraight UMETA(DisplayName="Continue Straight"),
	HoldLastKnownPoint UMETA(DisplayName="Hold Last Known Point"),
	Expire UMETA(DisplayName="Expire")
};

/**
 * 명중 판정과 분리된 유도 종료·실패 진단 사유입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileMissReason : uint8
{
	None UMETA(DisplayName="None"),
	GuidanceDisabled UMETA(DisplayName="Guidance Disabled"),
	InvalidInput UMETA(DisplayName="Invalid Input"),
	BelowMinimumGuidanceSpeed UMETA(DisplayName="Below Minimum Guidance Speed"),
	TargetLost UMETA(DisplayName="Target Lost"),
	SeekerFieldOfViewExceeded UMETA(DisplayName="Seeker Field Of View Exceeded"),
	LockBreakAngleExceeded UMETA(DisplayName="Lock Break Angle Exceeded"),
	Overshoot UMETA(DisplayName="Overshoot"),
	LifeExpired UMETA(DisplayName="Life Expired")
};

/**
 * ProjectileData가 소유할 물리 제한형 Guidance 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuideConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(DisplayName="미사일 유도 사용 (bUseGuidance)", ToolTip="True이고 GuideMode가 None이 아닐 때 후속 MissileGuideComp가 Guidance Command를 계산할 수 있습니다. 기본값 False에서는 기존 Velocity를 변경하지 않습니다."))
	bool bUseGuidance = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(EditCondition="bUseGuidance", DisplayName="유도 모드 (GuideMode)", ToolTip="TargetActor, LaserPoint, InertialPoint 또는 DataLink 중 관측 정보 출처입니다."))
	ECFMissileGuideMode GuideMode = ECFMissileGuideMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(EditCondition="bUseGuidance", DisplayName="목표 상실 정책 (LostTargetPolicy)", ToolTip="목표 참조 또는 신호가 무효화됐을 때 직진, 마지막 지점 유지 또는 만료 중 어떤 정책을 사용할지 정의합니다."))
	ECFMissileLostTargetPolicy LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="10.0", EditCondition="bUseGuidance", DisplayName="비례항법 계수 (NavigationConstant)", ToolTip="시선 각속도와 접근 속도로 요구 횡가속도를 계산할 때 사용할 비례항법 계수입니다."))
	float NavigationConstant = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="720.0", Units="deg/s", EditCondition="bUseGuidance", DisplayName="최대 선회율 deg/s (MaximumTurnRateDegPerSec)", ToolTip="현재 속도 방향이 초당 회전할 수 있는 최대 각도입니다. 위치나 Velocity를 목표 방향으로 즉시 덮어쓰지 않습니다."))
	float MaximumTurnRateDegPerSec = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm/s^2", EditCondition="bUseGuidance", DisplayName="최대 횡가속도 cm/s² (MaximumLateralAccelerationCmPerSecSq)", ToolTip="진행 방향에 수직으로 적용할 수 있는 Guidance 가속도의 최대 크기입니다."))
	float MaximumLateralAccelerationCmPerSecSq = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.001", ClampMax="10.0", Units="s", EditCondition="bUseGuidance", DisplayName="유도 응답 시간 초 (GuidanceResponseTimeSeconds)", ToolTip="후속 GuideComp가 Command 필터링에 사용할 응답 시간입니다. MG-P0-00 수학 함수는 제한값 계약만 제공하고 시간 필터를 적용하지 않습니다."))
	float GuidanceResponseTimeSeconds = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm/s", EditCondition="bUseGuidance", DisplayName="최소 유도 속도 cm/s (MinimumGuidanceSpeedCmPerSec)", ToolTip="이 속도보다 느리면 선회율과 횡가속도 계산을 유효 Guidance Command로 처리하지 않습니다."))
	float MinimumGuidanceSpeedCmPerSec = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="360.0", Units="deg", EditCondition="bUseGuidance", DisplayName="탐색기 시야각 deg (SeekerFieldOfViewDeg)", ToolTip="미사일 전방을 기준으로 탐색기가 관측 가능한 전체 원뿔 각도입니다. 실제 Lock 판정은 MG-P0-04에서 연결합니다."))
	float SeekerFieldOfViewDeg = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg", EditCondition="bUseGuidance", DisplayName="Lock 해제 각도 deg (LockBreakAngleDeg)", ToolTip="이미 추적 중인 목표가 이 각도보다 멀어졌을 때 Lock 상실을 검토합니다. 실제 수명 판정은 MG-P0-04에서 연결합니다."))
	float LockBreakAngleDeg = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="30.0", Units="s", EditCondition="bUseGuidance", DisplayName="목표 상실 유예 시간 초 (TargetLostGraceTimeSeconds)", ToolTip="목표 관측이 끊긴 뒤 LostTargetPolicy를 확정하기 전까지 유지할 수 있는 유예 시간입니다."))
	float TargetLostGraceTimeSeconds = 0.25f;

	// [v1.1.0] 기존 단일 각도 판정과 신규 상태형 Seeker 중 어떤 모델을 사용할지 선택합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Seeker", meta=(EditCondition="bUseGuidance", DisplayName="탐색기 모델 (SeekerModel)", ToolTip="Legacy Single Gate는 기존 SeekerFieldOfViewDeg와 LockBreakAngleDeg의 더 엄격한 각도를 그대로 사용합니다. Stateful은 최초 획득·추적 유지·상실 유예·재포착 상태를 분리합니다."))
	ECFMissileSeekerModel SeekerModel = ECFMissileSeekerModel::LegacySingleGate;

	// [v1.1.0] Target Actor 운동 정보를 직접 읽을지 일정 주기 위치 샘플로 추정할지 선택합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Observation", meta=(EditCondition="bUseGuidance", DisplayName="목표 관측 방식 (TargetObservationMode)", ToolTip="Direct Actor Kinematics는 기존처럼 Actor 위치와 속도를 직접 사용합니다. Sampled Position Estimate는 일정 주기 위치 샘플로 목표 속도를 추정합니다."))
	ECFMissileTargetObservationMode TargetObservationMode = ECFMissileTargetObservationMode::DirectActorKinematics;

	// [v1.2.0] 관측된 Target 상태를 조향 명령으로 바꿀 Guidance Law입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|GuidanceLaw", meta=(EditCondition="bUseGuidance", DisplayName="유도 방식 (GuidanceLaw)", ToolTip="비례항법은 상대운동을 이용한 고급 교차 유도, 단순 추적은 마지막 실제 관측 위치 직접 추적, 제한 선행 추적은 마지막 실제 관측 위치에 제한된 속도 선행량을 더해 추적합니다."))
	ECFMissileGuidanceLaw GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;

	// [v1.3.0] 기존 Flight Guidance Window 호환과 독립 시간·거리 활성화 중 이번 미사일이 사용할 방식을 선택합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Activation", meta=(EditCondition="bUseGuidance", DisplayName="유도 활성화 방식 (GuidanceActivationMode)", ToolTip="Follow Flight Guidance Window는 기존처럼 MissileFlightComp의 유도 구간을 따릅니다. Independent는 발사 후 시간과 분리 거리 조건을 모두 만족하면 Flight Guidance Window와 독립적으로 유도를 활성화하고 그 activation 동안 유지합니다."))
	ECFMissileGuidanceActivationMode GuidanceActivationMode = ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow;

	// [v1.3.0] Independent Guidance Activation에서 발사 후 유도를 허용할 최소 경과 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Activation", meta=(ClampMin="0.0", ClampMax="30.0", Units="s", EditCondition="bUseGuidance && GuidanceActivationMode == ECFMissileGuidanceActivationMode::Independent", DisplayName="유도 활성화 지연 시간 초 (GuidanceActivationDelaySeconds)", ToolTip="0이면 시간 조건을 즉시 만족합니다. 0보다 크면 MissileFlightComp의 ElapsedFlightTimeSeconds가 이 값 이상일 때 시간 조건을 만족합니다."))
	float GuidanceActivationDelaySeconds = 0.0f;

	// [v1.3.0] Independent Guidance Activation에서 발사 지점으로부터 요구할 최소 분리 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Activation", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm", EditCondition="bUseGuidance && GuidanceActivationMode == ECFMissileGuidanceActivationMode::Independent", DisplayName="유도 활성화 분리 거리 cm (GuidanceActivationDistanceCm)", ToolTip="0이면 거리 조건을 즉시 만족합니다. 0보다 크면 MissileFlightComp의 DistanceFromReleaseCm가 이 값 이상일 때 거리 조건을 만족합니다. 한 번 만족한 Independent activation은 U-turn으로 거리가 다시 줄어도 닫히지 않습니다."))
	float GuidanceActivationDistanceCm = 0.0f;

	// [v1.2.0] LeadPursuit가 필터된 Target 속도를 얼마만큼 미래로 투영할지 결정하는 선행 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|GuidanceLaw", meta=(ClampMin="0.0", ClampMax="10.0", Units="s", EditCondition="bUseGuidance && GuidanceLaw == ECFMissileGuidanceLaw::LeadPursuit", DisplayName="선행 시간 초 (LeadTimeSeconds)", ToolTip="제한 선행 추적에서 마지막 실제 관측 위치에 더할 Target 속도 선행 시간을 정합니다. EstimatedTargetLocation을 다시 기준점으로 사용하지 않습니다."))
	float LeadTimeSeconds = 0.20f;

	// [v1.2.0] LeadPursuit가 마지막 실제 관측 위치에서 앞쪽으로 만들 수 있는 최대 선행 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|GuidanceLaw", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm", EditCondition="bUseGuidance && GuidanceLaw == ECFMissileGuidanceLaw::LeadPursuit", DisplayName="최대 선행 거리 cm (MaxLeadDistanceCm)", ToolTip="제한 선행 추적의 FilteredTargetVelocityEstimate * LeadTimeSeconds 오프셋 크기를 이 거리 이하로 제한합니다."))
	float MaxLeadDistanceCm = 2000.0f;

	// [v1.1.0] Stateful Seeker가 목표를 잃은 뒤 같은 Launch Target Snapshot을 재포착할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Seeker", meta=(EditCondition="bUseGuidance && SeekerModel == ECFMissileSeekerModel::Stateful", DisplayName="재포착 방식 (ReacquisitionMode)", ToolTip="None은 상실 유예가 끝나면 최종 상실합니다. Forward Cone은 기존 Launch Target Snapshot과 같은 Actor만 전방 재포착 원뿔 안에서 다시 받아들입니다. 주변 Actor 자동 Retarget은 하지 않습니다."))
	ECFMissileReacquisitionMode ReacquisitionMode = ECFMissileReacquisitionMode::None;

	// [v1.1.0] Sampled Position Estimate에서 실제 Target 위치를 새로 읽는 최소 관측 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Observation", meta=(ClampMin="0.001", ClampMax="10.0", Units="s", EditCondition="bUseGuidance && TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate", DisplayName="목표 관측 간격 초 (TargetObservationIntervalSeconds)", ToolTip="Sampled Position Estimate에서 Target Actor의 새 위치를 관측하는 간격입니다. 값이 클수록 목표 기동 변화에 늦게 반응합니다."))
	float TargetObservationIntervalSeconds = 0.08f;

	// [v1.1.0] 위치 차분으로 얻은 목표 속도 추정값을 얼마나 빠르게 따라갈지 결정하는 응답 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Observation", meta=(ClampMin="0.001", ClampMax="10.0", Units="s", EditCondition="bUseGuidance && TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate", DisplayName="목표 속도 추정 응답 시간 초 (TargetVelocityEstimateResponseTimeSeconds)", ToolTip="새 위치 샘플에서 계산한 목표 속도 추정값을 필터링할 응답 시간입니다. 값이 클수록 급격한 목표 방향 변화 추정이 늦어집니다."))
	float TargetVelocityEstimateResponseTimeSeconds = 0.20f;

	// [v1.1.0] Stateful Seeker가 Launch Target Snapshot을 처음 Tracking으로 받아들일 수 있는 중심선 기준 반각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Seeker", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg", EditCondition="bUseGuidance && SeekerModel == ECFMissileSeekerModel::Stateful", DisplayName="최초 획득 반각 deg (AcquisitionConeHalfAngleDeg)", ToolTip="미사일 진행 중심선에서 목표까지 측정하는 반각입니다. 전체 FOV 각도가 아닙니다. Stateful에서는 Tracking 반각과 독립적으로 0~180도 범위에서 설정할 수 있습니다."))
	float AcquisitionConeHalfAngleDeg = 30.0f;

	// [v1.1.0] Stateful Seeker가 이미 획득한 Target을 계속 Tracking할 수 있는 중심선 기준 반각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Seeker", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg", EditCondition="bUseGuidance && SeekerModel == ECFMissileSeekerModel::Stateful", DisplayName="추적 유지 반각 deg (TrackingConeHalfAngleDeg)", ToolTip="미사일 진행 중심선에서 목표까지 측정하는 추적 유지 반각입니다. 전체 FOV 각도가 아닙니다. Acquisition과 Reacquisition 반각을 강제로 축소하지 않는 독립 값입니다."))
	float TrackingConeHalfAngleDeg = 60.0f;

	// [v1.1.0] ForwardCone 재포착에서 같은 Launch Target Snapshot을 다시 받아들일 수 있는 중심선 기준 반각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Seeker", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg", EditCondition="bUseGuidance && SeekerModel == ECFMissileSeekerModel::Stateful && ReacquisitionMode == ECFMissileReacquisitionMode::ForwardCone", DisplayName="재포착 반각 deg (ReacquisitionConeHalfAngleDeg)", ToolTip="미사일 진행 중심선에서 같은 Launch Target Snapshot까지 측정하는 재포착 반각입니다. 전체 FOV 각도가 아닙니다. Stateful에서는 Tracking 반각과 독립적으로 0~180도 범위에서 설정할 수 있습니다."))
	float ReacquisitionConeHalfAngleDeg = 45.0f;

	// [v1.1.0] ForwardCone 재포착을 시도할 수 있는 최대 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance|Seeker", meta=(ClampMin="0.0", ClampMax="30.0", Units="s", EditCondition="bUseGuidance && SeekerModel == ECFMissileSeekerModel::Stateful && ReacquisitionMode == ECFMissileReacquisitionMode::ForwardCone", DisplayName="재포착 시도 시간 초 (ReacquisitionTimeSeconds)", ToolTip="TargetLostGrace가 끝난 뒤 Forward Cone 재포착을 시도할 수 있는 최대 시간입니다. 0이면 재포착 상태에 머물지 않고 즉시 최종 상실합니다."))
	float ReacquisitionTimeSeconds = 0.50f;

	// [v1.0.0] Guidance가 실제로 활성화 가능한 설정인지 반환합니다.
	bool IsGuidanceEnabled() const
	{
		return bUseGuidance && GuideMode != ECFMissileGuideMode::None;
	}

	float GetEffectiveNavigationConstant() const
	{
		return FMath::IsFinite(NavigationConstant)
			? FMath::Clamp(NavigationConstant, 0.0f, 10.0f)
			: 0.0f;
	}

	float GetEffectiveMaximumTurnRateDegPerSec() const
	{
		return FMath::IsFinite(MaximumTurnRateDegPerSec)
			? FMath::Clamp(MaximumTurnRateDegPerSec, 0.0f, 720.0f)
			: 0.0f;
	}

	float GetEffectiveMaximumLateralAccelerationCmPerSecSq() const
	{
		return FMath::IsFinite(MaximumLateralAccelerationCmPerSecSq)
			? FMath::Clamp(MaximumLateralAccelerationCmPerSecSq, 0.0f, 1000000.0f)
			: 0.0f;
	}

	float GetEffectiveGuidanceResponseTimeSeconds() const
	{
		return FMath::IsFinite(GuidanceResponseTimeSeconds)
			? FMath::Clamp(GuidanceResponseTimeSeconds, 0.001f, 10.0f)
			: 0.001f;
	}

	float GetEffectiveMinimumGuidanceSpeedCmPerSec() const
	{
		return FMath::IsFinite(MinimumGuidanceSpeedCmPerSec)
			? FMath::Clamp(MinimumGuidanceSpeedCmPerSec, 0.0f, 1000000.0f)
			: 0.0f;
	}

	float GetEffectiveSeekerFieldOfViewDeg() const
	{
		return FMath::IsFinite(SeekerFieldOfViewDeg)
			? FMath::Clamp(SeekerFieldOfViewDeg, 0.0f, 360.0f)
			: 0.0f;
	}

	float GetEffectiveLockBreakAngleDeg() const
	{
		return FMath::IsFinite(LockBreakAngleDeg)
			? FMath::Clamp(LockBreakAngleDeg, 0.0f, 180.0f)
			: 0.0f;
	}

	float GetEffectiveTargetLostGraceTimeSeconds() const
	{
		return FMath::IsFinite(TargetLostGraceTimeSeconds)
			? FMath::Clamp(TargetLostGraceTimeSeconds, 0.0f, 30.0f)
			: 0.0f;
	}

	// [v1.3.0] Independent Guidance Activation 지연 시간의 안전 보정값을 반환합니다.
	float GetEffectiveGuidanceActivationDelaySeconds() const
	{
		return FMath::IsFinite(GuidanceActivationDelaySeconds)
			? FMath::Clamp(GuidanceActivationDelaySeconds, 0.0f, 30.0f)
			: 0.0f;
	}

	// [v1.3.0] Independent Guidance Activation 분리 거리의 안전 보정값을 반환합니다.
	float GetEffectiveGuidanceActivationDistanceCm() const
	{
		return FMath::IsFinite(GuidanceActivationDistanceCm)
			? FMath::Clamp(GuidanceActivationDistanceCm, 0.0f, 1000000.0f)
			: 0.0f;
	}

	// [v1.2.0] LeadPursuit 선행 시간의 안전 보정값을 반환합니다.
	float GetEffectiveLeadTimeSeconds() const
	{
		return FMath::IsFinite(LeadTimeSeconds)
			? FMath::Clamp(LeadTimeSeconds, 0.0f, 10.0f)
			: 0.0f;
	}

	// [v1.2.0] LeadPursuit 최대 선행 거리의 안전 보정값을 반환합니다.
	float GetEffectiveMaxLeadDistanceCm() const
	{
		return FMath::IsFinite(MaxLeadDistanceCm)
			? FMath::Clamp(MaxLeadDistanceCm, 0.0f, 1000000.0f)
			: 0.0f;
	}

	// [v1.1.0] Sampled Target 위치를 새로 관측할 안전 보정 간격을 반환합니다.
	float GetEffectiveTargetObservationIntervalSeconds() const
	{
		return FMath::IsFinite(TargetObservationIntervalSeconds)
			? FMath::Clamp(TargetObservationIntervalSeconds, 0.001f, 10.0f)
			: 0.001f;
	}

	// [v1.1.0] Target 속도 추정 필터의 안전 보정 응답 시간을 반환합니다.
	float GetEffectiveTargetVelocityEstimateResponseTimeSeconds() const
	{
		return FMath::IsFinite(TargetVelocityEstimateResponseTimeSeconds)
			? FMath::Clamp(TargetVelocityEstimateResponseTimeSeconds, 0.001f, 10.0f)
			: 0.001f;
	}

	// [v1.1.0] Stateful Tracking이 유지될 수 있는 중심선 기준 안전 보정 반각을 반환합니다.
	float GetEffectiveTrackingConeHalfAngleDeg() const
	{
		return FMath::IsFinite(TrackingConeHalfAngleDeg)
			? FMath::Clamp(TrackingConeHalfAngleDeg, 0.0f, 180.0f)
			: 0.0f;
	}

	// [v1.3.0] Stateful 최초 획득 반각을 다른 Seeker 반각과 결합하지 않고 0~180도로 안전 보정해 반환합니다.
	float GetEffectiveAcquisitionConeHalfAngleDeg() const
	{
		return FMath::IsFinite(AcquisitionConeHalfAngleDeg)
			? FMath::Clamp(AcquisitionConeHalfAngleDeg, 0.0f, 180.0f)
			: 0.0f;
	}

	// [v1.3.0] Stateful 재포착 반각을 다른 Seeker 반각과 결합하지 않고 0~180도로 안전 보정해 반환합니다.
	float GetEffectiveReacquisitionConeHalfAngleDeg() const
	{
		return FMath::IsFinite(ReacquisitionConeHalfAngleDeg)
			? FMath::Clamp(ReacquisitionConeHalfAngleDeg, 0.0f, 180.0f)
			: 0.0f;
	}

	// [v1.1.0] Stateful ForwardCone 재포착의 안전 보정 최대 시도 시간을 반환합니다.
	float GetEffectiveReacquisitionTimeSeconds() const
	{
		return FMath::IsFinite(ReacquisitionTimeSeconds)
			? FMath::Clamp(ReacquisitionTimeSeconds, 0.0f, 30.0f)
			: 0.0f;
	}

	// [v1.0.0] 원본 설정을 변경하지 않고 모든 수치 제한이 적용된 설정 복사본을 반환합니다.
	FCFMissileGuideConfig GetEffectiveConfig() const
	{
		FCFMissileGuideConfig EffectiveConfig = *this;
		if (!EffectiveConfig.bUseGuidance)
		{
			EffectiveConfig.GuideMode = ECFMissileGuideMode::None;
		}
		EffectiveConfig.NavigationConstant = GetEffectiveNavigationConstant();
		EffectiveConfig.MaximumTurnRateDegPerSec = GetEffectiveMaximumTurnRateDegPerSec();
		EffectiveConfig.MaximumLateralAccelerationCmPerSecSq = GetEffectiveMaximumLateralAccelerationCmPerSecSq();
		EffectiveConfig.GuidanceResponseTimeSeconds = GetEffectiveGuidanceResponseTimeSeconds();
		EffectiveConfig.MinimumGuidanceSpeedCmPerSec = GetEffectiveMinimumGuidanceSpeedCmPerSec();
		EffectiveConfig.SeekerFieldOfViewDeg = GetEffectiveSeekerFieldOfViewDeg();
		EffectiveConfig.LockBreakAngleDeg = GetEffectiveLockBreakAngleDeg();
		EffectiveConfig.TargetLostGraceTimeSeconds = GetEffectiveTargetLostGraceTimeSeconds();
		EffectiveConfig.GuidanceActivationDelaySeconds = GetEffectiveGuidanceActivationDelaySeconds();
		EffectiveConfig.GuidanceActivationDistanceCm = GetEffectiveGuidanceActivationDistanceCm();
		EffectiveConfig.LeadTimeSeconds = GetEffectiveLeadTimeSeconds();
		EffectiveConfig.MaxLeadDistanceCm = GetEffectiveMaxLeadDistanceCm();
		EffectiveConfig.TargetObservationIntervalSeconds = GetEffectiveTargetObservationIntervalSeconds();
		EffectiveConfig.TargetVelocityEstimateResponseTimeSeconds = GetEffectiveTargetVelocityEstimateResponseTimeSeconds();
		EffectiveConfig.TrackingConeHalfAngleDeg = GetEffectiveTrackingConeHalfAngleDeg();
		EffectiveConfig.AcquisitionConeHalfAngleDeg = GetEffectiveAcquisitionConeHalfAngleDeg();
		EffectiveConfig.ReacquisitionConeHalfAngleDeg = GetEffectiveReacquisitionConeHalfAngleDeg();
		EffectiveConfig.ReacquisitionTimeSeconds = GetEffectiveReacquisitionTimeSeconds();
		return EffectiveConfig;
	}
};

/**
 * 순수 Guidance 수학 함수가 한 단계 계산에 사용할 값 타입 입력입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuidanceInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector MissileLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector MissileVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector TargetVelocityEstimate = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	float DeltaSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	bool bHasTarget = false;
};

/**
 * 물리 제한을 적용하기 전·후 횡가속도와 선회율을 보존하는 순수 Guidance 결과입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuidanceCommand
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bCommandValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector RequestedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector AppliedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector LineOfSightAngularVelocityRadPerSec = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float ClosingSpeedCmPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float SeekerAngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float RequestedTurnRateDegPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float AppliedTurnRateDegPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bLimitedByLateralAcceleration = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bLimitedByTurnRate = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	ECFMissileMissReason InvalidReason = ECFMissileMissReason::None;
};

/**
 * 후속 MissileGuideComp가 Target·Command·상실 상태를 한 번에 노출할 Debug Snapshot입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuideSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	ECFMissileGuideMode GuideMode = ECFMissileGuideMode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bTargetValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float SeekerAngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector RequestedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector AppliedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float TargetLostTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	ECFMissileMissReason MissReason = ECFMissileMissReason::None;

	// [v1.1.0] 현재 활성 설정이 선택한 Seeker 모델입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="탐색기 모델 (SeekerModel)", ToolTip="현재 활성 MissileGuideConfig가 선택한 Legacy Single Gate 또는 Stateful Seeker 모델입니다."))
	ECFMissileSeekerModel SeekerModel = ECFMissileSeekerModel::LegacySingleGate;

	// [v1.1.0] Stateful Seeker가 현재 위치한 발사 후 추적 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="탐색기 상태 (SeekerState)", ToolTip="Inactive, Acquiring, Tracking, LostGrace, Reacquiring 또는 LostFinal 상태입니다. Legacy 모델에서는 Inactive 기본값을 유지할 수 있습니다."))
	ECFMissileSeekerState SeekerState = ECFMissileSeekerState::Inactive;

	// [v1.1.0] 현재 활성 설정이 선택한 Target 관측 모델입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="목표 관측 방식 (TargetObservationMode)", ToolTip="현재 Target Actor 운동 정보를 직접 읽는지, 위치 샘플 기반 추정값을 사용하는지 나타냅니다."))
	ECFMissileTargetObservationMode TargetObservationMode = ECFMissileTargetObservationMode::DirectActorKinematics;

	// [v1.2.0] 현재 활성 설정이 선택한 Guidance Law입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="유도 방식 (GuidanceLaw)", ToolTip="현재 미사일이 ProportionalNavigation, PurePursuit 또는 LeadPursuit 중 어떤 알고리즘으로 조향 명령을 만드는지 나타냅니다."))
	ECFMissileGuidanceLaw GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;

	// [v1.3.0] 현재 활성 설정이 선택한 Guidance Activation 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="유도 활성화 방식 (GuidanceActivationMode)", ToolTip="기존 Flight Guidance Window를 따르는지, 발사 후 시간·거리 조건으로 독립 활성화하는지 나타냅니다."))
	ECFMissileGuidanceActivationMode GuidanceActivationMode = ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow;

	// [v1.3.0] 현재 activation에서 Guidance가 실제로 열렸는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="유도 활성화 만족 (bGuidanceActivationSatisfied)", ToolTip="Follow 모드에서는 현재 Flight Guidance Window 상태, Independent 모드에서는 시간·거리 AND 조건을 한 번 만족해 latch된 상태를 나타냅니다."))
	bool bGuidanceActivationSatisfied = false;

	// [v1.2.0] 현재 Guidance Law가 이번 Step에서 실제로 조준한 월드 지점입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="유도 조준 지점 (GuidanceAimPoint)", ToolTip="PurePursuit는 마지막 실제 관측 위치, LeadPursuit는 제한된 선행 지점, ProportionalNavigation은 현재 Sensor Truth 위치를 기록합니다."))
	FVector GuidanceAimPoint = FVector::ZeroVector;

	// [v1.2.0] PN이 아직 접근 기하를 만들지 못해 물리 제한형 Course Capture를 사용 중인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="코스 포착 활성 (bCourseCaptureActive)", ToolTip="ProportionalNavigation에서 Target이 뒤쪽 또는 비접근 기하라 PN 대신 Target 방향으로 물리 제한형 선회를 먼저 수행하면 True입니다."))
	bool bCourseCaptureActive = false;

	// [v1.3.0] Stateful Overshoot가 실제 접근 성립 이후 판정을 허용하도록 arm됐는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="오버슈트 판정 준비 (bOvershootArmed)", ToolTip="Stateful Seeker에서 Target 거리가 감소하면서 접근 속도가 양수인 상태가 한 번 성립하면 True가 됩니다. 초기 rear U-turn 동안에는 False를 유지합니다."))
	bool bOvershootArmed = false;

	// [v1.1.0] Sampled 관측에서 실제 Target Actor로부터 마지막으로 취득한 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="마지막 관측 목표 위치 (LastObservedTargetLocation)", ToolTip="Sampled 관측이 Target Actor에서 실제로 마지막 취득한 월드 위치입니다."))
	FVector LastObservedTargetLocation = FVector::ZeroVector;

	// [v1.1.0] 현재 Seeker·PN·Overshoot 판단이 소비할 Sensor Truth 목표 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="추정 목표 위치 (EstimatedTargetLocation)", ToolTip="현재 관측 모델이 Guidance 계산에 제공하는 Sensor Truth 목표 위치입니다."))
	FVector EstimatedTargetLocation = FVector::ZeroVector;

	// [v1.1.0] 위치 샘플 차분에서 계산하고 응답 시간으로 필터링한 목표 속도 추정값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="필터된 목표 속도 추정 (FilteredTargetVelocityEstimate)", ToolTip="Sampled 위치 차분으로 계산한 뒤 설정된 응답 시간으로 필터링한 목표 속도 추정값입니다."))
	FVector FilteredTargetVelocityEstimate = FVector::ZeroVector;

	// [v1.1.0] 마지막 실제 Target 위치 관측 이후 누적된 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="관측 경과 시간 초 (ObservationAgeSeconds)", ToolTip="마지막 실제 Target 위치 샘플 이후 경과한 시간입니다. Sample 간격과 위치 외삽이 같은 시간축을 사용합니다."))
	float ObservationAgeSeconds = 0.0f;

	// [v1.1.0] 현재 Reacquiring 상태에서 누적된 재포착 시도 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="재포착 경과 시간 초 (ReacquisitionElapsedTimeSeconds)", ToolTip="Stateful Seeker가 현재 Reacquiring 상태에서 같은 Launch Target Snapshot을 다시 찾기 위해 소비한 시간입니다."))
	float ReacquisitionElapsedTimeSeconds = 0.0f;

	// [v1.1.0] 현재 activation에서 유효한 Sensor 위치 seed 또는 sample을 한 번 이상 확보했는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance", meta=(DisplayName="유효 관측 보유 (bHasValidObservation)", ToolTip="현재 발사 activation에서 유효한 Target 위치 관측 seed 또는 sample을 한 번 이상 확보했으면 True입니다."))
	bool bHasValidObservation = false;
};
