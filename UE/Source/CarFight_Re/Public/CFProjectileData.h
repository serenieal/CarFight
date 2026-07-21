// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-07-14
// Description: CarFight 차량 발사체 DataAsset
// Scope: WeaponData가 참조할 최소 ProjectileData와 디버그 요약 함수를 제공합니다.
// Changelog:
// - v1.5.0: ProjectileMovement Sweep / Sub-step, 보조 연속 Sphere Sweep, CCD 설정을 탄종별 데이터로 추가.
// - v1.4.0: HitScan / Laser용 가상 ProjectileData까지 포함하는 DamageData 단일 소유 정책을 명시.
// - v1.3.0: 발사체별 기본 DamageData 직접 참조를 추가하고 디버그 요약에 DamageData 연결 상태를 포함.
// - v1.2.0: 공통 Projectile Actor용 메시 설정을 추가하고 ProjectileActorClass 타입을 CFProjectileActor로 제한.
// - v1.1.0: ProjectileActorClass 연결 여부를 Projectile Actor 스폰 준비 판정으로 노출.
// - v1.0.0: ProjectileData / DamageData 분리의 첫 단계로 최소 발사체 DataAsset 타입을 추가.
// Migration:
// - WeaponData.DefaultProjectileData에서 이 DataAsset을 선택적으로 참조한다.
// - ProjectileActorClass가 비어 있거나 FireMode가 Projectile이 아니면 기존 Dummy HitScan / FireOrigin / Cooldown 흐름은 유지한다.
// - ProjectileActorClass는 CFProjectileActor 기반 Blueprint를 지정한다.
// - HitScan / Laser처럼 실제 Actor를 스폰하지 않는 무기는 ProjectileActorClass를 비운 가상 ProjectileData를 사용한다.
// - DefaultDamageData는 DamageData의 단일 직접 참조 슬롯이며, 비어 있으면 DamageProfileId를 디버그 fallback으로만 표시한다.
// - 기존 ProjectileData 자산은 신규 연속 충돌 필드의 C++ 기본값을 사용하므로 이번 코드 작업에서 .uasset 저장이 필요하지 않다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileActor.h"
#include "Engine/DataAsset.h"
#include "CFProjectileData.generated.h"

class UStaticMesh;
class UCFDamageData;

/**
 * 무기에서 분리된 최소 발사체 데이터입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFProjectileData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 발사체 데이터 값을 초기화합니다.
	UCFProjectileData();

	// [v1.5.0] 디버그 패널에 표시할 발사체 데이터, 연속 충돌, DamageData 연결 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData", meta=(DisplayName="발사체 요약 생성 (Build Projectile Summary)", ToolTip="디버그 패널과 로그에 표시할 핵심 발사체 데이터, 연속 충돌 설정, 스폰 준비 상태를 생성합니다."))
	FString BuildProjectileSummary() const;

	// [v1.1.0] Projectile Actor 스폰 후보 클래스가 지정되어 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData", meta=(DisplayName="발사체 Actor 클래스 지정 여부 (Has Projectile Actor Class)", ToolTip="ProjectileActorClass가 지정되어 실제 Projectile Actor 전환 후보인지 반환합니다. FireMode가 Projectile이어야 실제 스폰 경로로 사용됩니다."))
	bool HasProjectileActorClass() const;

	// [v1.0.0] 발사체 데이터를 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Identity", meta=(DisplayName="발사체 ID (ProjectileId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 발사체를 식별할 이름입니다. 예: ProtoShell"))
	FName ProjectileId = TEXT("ProtoShell");

	// [v1.0.0] 발사체가 생성될 때 사용할 초기 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(ClampMin="0.0", DisplayName="초기 속도 (InitialSpeed)", ToolTip="Projectile Actor 전환 뒤 발사체가 처음 이동할 속도입니다. Dummy HitScan 단계에서는 표시용 기준값입니다."))
	float InitialSpeed = 6000.0f;

	// [v1.0.0] 발사체가 중력 영향을 받을지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(DisplayName="중력 영향 여부 (bAffectedByGravity)", ToolTip="True이면 Projectile Actor 전환 뒤 발사체 이동에 중력 보정을 적용합니다. Dummy HitScan 단계에서는 표시용 기준값입니다."))
	bool bAffectedByGravity = true;

	// [v1.0.0] 발사체에 적용할 중력 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(ClampMin="0.0", EditCondition="bAffectedByGravity", DisplayName="중력 배율 (GravityScale)", ToolTip="Projectile Actor 전환 뒤 중력 크기를 조절할 배율입니다. 중력 영향이 꺼져 있으면 무시합니다."))
	float GravityScale = 1.0f;

	// [v1.0.0] 발사체가 자동 제거되기 전까지 유지될 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(ClampMin="0.0", DisplayName="수명 초 (LifeTimeSeconds)", ToolTip="Projectile Actor 전환 뒤 발사체가 월드에 유지될 최대 시간입니다. Dummy HitScan 단계에서는 최대 비행 시간 기준값입니다."))
	float LifeTimeSeconds = 3.0f;

	// [v1.0.0] 발사체 충돌 판정에 사용할 반경입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Collision", meta=(ClampMin="0.0", DisplayName="충돌 반경 (CollisionRadius)", ToolTip="Projectile Actor 전환 뒤 발사체 충돌 판정에 사용할 반경입니다. Dummy HitScan 단계에서는 표시용 기준값입니다."))
	float CollisionRadius = 8.0f;

	// [v1.5.0] ProjectileMovement가 이동 구간을 Sweep으로 검사할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Collision", meta=(DisplayName="이동 Sweep 충돌 사용 (bUseSweepCollision)", ToolTip="True이면 ProjectileMovement가 이전 위치에서 다음 위치까지 충돌 형상을 Sweep해 첫 Blocking Hit을 검사합니다. 실제 충돌 발사체의 기본값은 True입니다."))
	bool bUseSweepCollision = true;

	// [v1.5.0] 한 프레임 이동을 더 작은 시뮬레이션 단계로 분할할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(DisplayName="강제 Sub-step 사용 (bForceSubStepping)", ToolTip="True이면 고속 또는 중력 발사체의 한 프레임 이동을 더 작은 단계로 나누어 충돌 누락과 궤적 오차를 줄입니다."))
	bool bForceSubStepping = true;

	// [v1.5.0] ProjectileMovement가 한 번에 처리할 최대 시뮬레이션 시간 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(ClampMin="0.001", ClampMax="0.050", DisplayName="최대 시뮬레이션 시간 간격 (MaxSimulationTimeStep)", ToolTip="Sub-step 한 단계의 최대 시간입니다. 기본값은 1/120초입니다. 값이 작을수록 정밀하지만 비용이 증가합니다."))
	float MaxSimulationTimeStep = 0.008333f;

	// [v1.5.0] 한 프레임에서 허용할 최대 ProjectileMovement 시뮬레이션 반복 횟수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(ClampMin="1", ClampMax="32", DisplayName="최대 시뮬레이션 반복 (MaxSimulationIterations)", ToolTip="한 프레임에서 Sub-step을 처리할 최대 횟수입니다. 기본값은 8입니다."))
	int32 MaxSimulationIterations = 8;

	// [v1.5.0] ProjectileMovement 이후 이전 위치부터 현재 위치까지 보조 Sphere Sweep을 수행할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Collision", meta=(DisplayName="보조 연속 Sweep 사용 (bUseSupplementalContinuousSweep)", ToolTip="True이면 ProjectileMovement가 놓친 잔여 터널링을 막기 위해 이전 위치부터 현재 위치까지 CollisionRadius 기반 Sphere Sweep을 추가로 수행합니다."))
	bool bUseSupplementalContinuousSweep = true;

	// [v1.5.0] 충돌 Sphere의 CCD를 보조 안전장치로 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Collision", meta=(DisplayName="CCD 보조 사용 (bUseCCD)", ToolTip="True이면 충돌 Sphere에 CCD를 보조로 사용합니다. 주 해결책은 Sweep, Sub-step, 보조 연속 Sweep이며 기본값은 False입니다."))
	bool bUseCCD = false;

	// [v1.2.0] 실제 발사체 스폰에 사용할 CFProjectileActor 기반 클래스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Actor", meta=(DisplayName="발사체 Actor 클래스 (ProjectileActorClass)", ToolTip="Projectile 전환 뒤 FireOrigin 위치와 방향에서 스폰할 Actor 클래스입니다. 비어 있으면 Dummy HitScan fallback을 유지합니다."))
	TSubclassOf<ACFProjectileActor> ProjectileActorClass = nullptr;

	// [v1.2.0] 공통 Projectile Actor가 표시할 발사체 StaticMesh입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Visual", meta=(DisplayName="발사체 메시 (ProjectileStaticMesh)", ToolTip="공통 Projectile Actor가 표시할 StaticMesh입니다. 탄종별 외형은 이 필드로 바꿉니다."))
	TObjectPtr<UStaticMesh> ProjectileStaticMesh = nullptr;

	// [v1.2.0] 발사체 메시가 이동 방향에 맞게 보이도록 보정할 상대 회전입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Visual", meta=(DisplayName="발사체 메시 상대 회전 (ProjectileMeshRelativeRotation)", ToolTip="Projectile Actor 안에서 메시 방향을 보정할 상대 회전입니다. 메시의 앞 방향이 X축과 다르면 여기서 보정합니다."))
	FRotator ProjectileMeshRelativeRotation = FRotator::ZeroRotator;

	// [v1.2.0] 발사체 메시 표시 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Visual", meta=(DisplayName="발사체 메시 상대 스케일 (ProjectileMeshRelativeScale)", ToolTip="Projectile Actor 안에서 표시할 메시 크기입니다. 충돌 반경은 CollisionRadius가 따로 소유합니다."))
	FVector ProjectileMeshRelativeScale = FVector::OneVector;

	// [v1.0.0] 피격 표현 후보를 찾기 위한 임시 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Impact", meta=(DisplayName="피격 효과 ID (ImpactEffectId)", ToolTip="후속 VFX / SFX / Decal 연결 전 피격 표현 후보를 구분할 임시 ID입니다."))
	FName ImpactEffectId = TEXT("ProtoImpact");

	// [v1.4.0] 이 발사체 또는 가상 발사 데이터가 직접 사용할 기본 DamageData입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Damage", meta=(DisplayName="기본 피해 데이터 (DefaultDamageData)", ToolTip="이 발사체 또는 가상 HitScan / Laser 발사 데이터가 직접 사용할 DamageData입니다. DamageData 직접 참조 슬롯은 ProjectileData에만 둡니다."))
	TObjectPtr<UCFDamageData> DefaultDamageData = nullptr;

	// [v1.4.0] DamageData 미연결 상태를 확인하기 위한 디버그 fallback 피해 프로파일 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Damage", meta=(DisplayName="피해 프로파일 ID (DamageProfileId)", ToolTip="DamageData가 아직 연결되지 않았을 때 디버그 패널에 표시할 피해 프로파일 ID입니다. 실제 피해 계산의 소유권은 DefaultDamageData가 갖습니다."))
	FName DamageProfileId = TEXT("ProtoDirectHit");
};
