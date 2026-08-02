// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-02
// Description: CarFight 발사체 발사 인계 공용 타입
// Scope: 런처가 발사 순간 확정한 Transform, 초기 방향·월드 속도, 선택 Muzzle, 명령 목표, 유도 목표와 요청 식별값을 Projectile에 값으로 전달합니다.
// Changelog:
// - v1.2.0: 발사 순간 선택 대상을 미사일이 독립적으로 유지할 GuidanceTargetActor Snapshot 추가.
// - v1.1.0: Launch Context에 선택 Muzzle 소켓 이름·인덱스·설정 슬롯 수 스냅샷을 추가.
// - v1.0.0: Direct·AngledEjection·VerticalEjection Release Mode와 FCFProjectileLaunchContext 최초 추가.
// Migration:
// - 기존 SpawnTransform + LaunchDirection 호출은 Muzzle 미지정 Direct Launch Context를 생성하는 호환 Adapter를 사용합니다.
// - GuidanceTargetActor 기본값은 None이며 기존 Projectile·Rocket 활성화 결과를 변경하지 않습니다.
// - InheritedCarrierVelocity 기본값은 Zero이며 기존 직사 Projectile에 차량 속도를 새로 상속하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileLaunchTypes.generated.h"

class AActor;

/**
 * Projectile이 런처에서 분리되는 초기 방식을 구분합니다.
 */
UENUM(BlueprintType)
enum class ECFProjectileReleaseMode : uint8
{
	Direct UMETA(DisplayName="직접 발사 (Direct)"),
	AngledEjection UMETA(DisplayName="대각 사출 (Angled Ejection)"),
	VerticalEjection UMETA(DisplayName="수직 사출 (Vertical Ejection)")
};

/**
 * 런처가 발사 순간 확정해 Projectile Actor에 값으로 복사하는 발사 인계 데이터입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFProjectileLaunchContext
{
	GENERATED_BODY()

	// [v1.0.0] Projectile이 런처에서 분리되는 순간의 월드 Transform입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="발사 월드 Transform (LaunchTransform)", ToolTip="Projectile이 런처에서 분리되는 순간 적용할 월드 위치와 회전입니다. 활성화 뒤 런처 Transform 변경과 독립적으로 유지됩니다."))
	FTransform LaunchTransform = FTransform::Identity;

	// [v1.0.0] 런처가 제공한 초기 사출 또는 직접 발사 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="초기 발사 방향 (InitialLaunchDirection)", ToolTip="발사 순간 Projectile의 자세와 비유도 Rocket 고정 추진 방향에 사용할 정규화 월드 방향입니다."))
	FVector InitialLaunchDirection = FVector::ForwardVector;

	// [v1.0.0] 사출 속도와 선택적 플랫폼 속도 상속을 합친 초기 월드 Velocity입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="초기 발사 월드 속도 (InitialLaunchVelocity)", ToolTip="ProjectileMovement에 발사 순간 직접 적용할 월드 Velocity입니다. 무효하거나 0이면 초기 방향과 ProjectileData.InitialSpeed로 복구합니다."))
	FVector InitialLaunchVelocity = FVector::ZeroVector;

	// [v1.0.0] 발사 플랫폼에서 상속한 월드 Velocity 성분입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="상속 플랫폼 속도 (InheritedCarrierVelocity)", ToolTip="차량 등 발사 플랫폼에서 상속한 월드 Velocity 성분입니다. LM-P0-01 Direct 호환 경로에서는 Zero입니다."))
	FVector InheritedCarrierVelocity = FVector::ZeroVector;

		// [v1.0.0] 플레이어 조준 또는 센서가 발사 순간 지시한 월드 목표 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="명령 목표 위치 (CommandTargetLocation)", ToolTip="초기 발사 방향과 독립적으로 보존하는 플레이어 조준 또는 센서의 명령 목표 월드 위치입니다. 현재 Direct Projectile은 이동 계산에 사용하지 않습니다."))
	FVector CommandTargetLocation = FVector::ZeroVector;

	// [v1.2.0] 발사 순간 선택되어 이미 발사된 미사일이 독립적으로 유지할 유도 목표 Actor입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="유도 목표 Actor (GuidanceTargetActor)", ToolTip="발사 순간 TargetSelectComp에서 복사한 목표 Actor입니다. 발사 후 차량의 선택 대상이 변경돼도 이미 발사된 미사일의 목표는 바뀌지 않습니다."))
	TObjectPtr<AActor> GuidanceTargetActor = nullptr;

		// [v1.1.0] 이번 Projectile을 실제로 발사한 Muzzle 소켓 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="발사 Muzzle 소켓 이름 (MuzzleSocketName)", ToolTip="발사 순간 선택된 Pitch 메쉬 Muzzle 소켓 이름의 복사본입니다. 기존 직접 API 호출은 None일 수 있습니다."))
	FName MuzzleSocketName = NAME_None;

	// [v1.1.0] 이번 Projectile을 발사한 MuzzleSocketNames 배열 인덱스입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="발사 Muzzle 인덱스 (MuzzleSocketIndex)", ToolTip="발사 순간 선택된 MuzzleSocketNames 배열 인덱스입니다. 기존 단일 Muzzle는 0, 미지정은 -1입니다."))
	int32 MuzzleSocketIndex = INDEX_NONE;

	// [v1.1.0] 발사 순간 선택 Muzzle가 순환하던 설정 슬롯 전체 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="설정 Muzzle 슬롯 수 (MuzzleSocketCount)", ToolTip="발사 순간 스냅샷으로 복사된 MuzzleSocketNames 설정 슬롯 전체 개수입니다. 기존 단일 Muzzle는 1입니다."))
	int32 MuzzleSocketCount = 0;

	// [v1.0.0] 이번 Projectile의 발사 분리 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="발사 분리 모드 (ReleaseMode)", ToolTip="Direct, AngledEjection 또는 VerticalEjection 중 이번 발사 방식입니다. LM-P0-02에서는 Direct만 사용합니다."))
	ECFProjectileReleaseMode ReleaseMode = ECFProjectileReleaseMode::Direct;

	// [v1.0.0] 차량 Fire Command와 Projectile 활성화를 추적할 증가형 요청 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="발사 요청 ID (FireRequestId)", ToolTip="Vehicle Fire Command에서 생성해 이번 Projectile 활성화까지 전달한 요청 ID입니다."))
	int32 FireRequestId = 0;

	// [v1.0.0] 이번 발사를 요청한 무기 또는 장착 그룹 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(DisplayName="무기 그룹 ID (WeaponGroupId)", ToolTip="이번 발사를 요청한 무기 그룹 또는 장착 프로파일 식별자입니다."))
	FName WeaponGroupId = NAME_None;
};
