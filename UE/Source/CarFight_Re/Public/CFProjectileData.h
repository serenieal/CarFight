// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.9.0
// Date: 2026-07-30
// Description: CarFight 차량 발사체 DataAsset
// Scope: WeaponData가 참조할 ProjectileData, 추진·비행 FX, Projectile 요격과 기본 비활성 Missile Foundation 설정을 제공합니다.
// Changelog:
// - v1.9.0: 다른 발사 차량의 Projectile·Hitscan에 의한 P0 요격 가능 여부와 요격 시 Impact FX 요청 설정을 추가.
// - v1.8.0: MG-P0-00 MissileFlightConfig·MissileGuideConfig와 유효 설정·요약 Getter를 기본 비활성으로 추가.
// - v1.7.1: RelativeTransform Scale을 소켓·Fallback 공통 독립 FX Scale로 명확화.
// - v1.7.0: 비유도 로켓의 점화 지연, 연소 시간, 추진 가속도와 최대 추진 속도를 위한 PropulsionConfig 추가.
// - v1.6.0: Trail·Thruster Niagara, 메시 소켓 우선과 Projectile Relative Fallback을 위한 공용 비행 FX 설정 추가.
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
// - TrailFxSettings와 ThrusterFxSettings는 기본 비활성이라 기존 ProjectileData를 재저장하지 않아도 이전 동작을 유지한다.
// - 메시 소켓이 없으면 RelativeTransform의 위치·회전을 CollisionComponent 기준 Fallback으로 사용한다.
// - RelativeTransform의 Scale은 소켓 사용 여부와 관계없이 메시 Scale과 분리된 독립 FX 배율로 적용한다.
// - PropulsionConfig.bUsePropulsion은 기본 false이므로 기존 ProjectileData는 InitialSpeed 고정 비행을 그대로 유지한다.
// - 추진이 활성화된 발사체는 InitialSpeed로 분리된 뒤 고정 발사 방향으로 가속하고, 연소 종료 후 기존 속도와 중력으로 관성 비행한다.
// - MissileFlightConfig.bUseMissileFlight와 MissileGuideConfig.bUseGuidance는 기본 false이므로 기존 Projectile·Rocket 이동과 모터 방향을 변경하지 않는다.
// - MG-P0-00은 데이터·수학 계약만 추가하며 ProjectileActor, ProjectileMovement, MotorComp와 TargetSelect에 런타임 연결하지 않는다.
// - bCanBeIntercepted와 bDetonateWhenIntercepted는 기본 true라 기존 ProjectileData를 재저장하지 않아도 다른 차량의 유효 Projectile·Hitscan 적중으로 요격·Impact FX가 가능하다.
// - 같은 차량의 Projectile은 탄종·Volley·FireRequest와 무관하게 Pool 런타임에서 서로 무시하므로 요격 설정을 소비하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileFlightTypes.h"
#include "CFMissileGuideTypes.h"
#include "CFProjectileMotorTypes.h"
#include "CFProjectileActor.h"
#include "Engine/DataAsset.h"
#include "CFProjectileData.generated.h"

class UStaticMesh;
class UCFCombatFxData;
class UCFDamageData;
class UNiagaraSystem;

/**
 * 지속형 Projectile FX를 배치할 기준 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFProjectileFxAttachMode : uint8
{
	ProjectileRelative UMETA(DisplayName="Projectile Relative"),
	MeshSocketWithFallback UMETA(DisplayName="Mesh Socket With Fallback")
};

/**
 * Trail 또는 추진 화염처럼 Projectile 활성 시간 동안 유지할 Niagara 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFProjectileAttachedFxSettings
{
	GENERATED_BODY()

	// [v1.6.0] 이 지속형 FX 슬롯을 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(DisplayName="비행 FX 사용 (bEnabled)", ToolTip="True이면 이 슬롯의 Niagara를 Projectile 활성 시간 동안 재생합니다. False이면 기존 발사체 동작을 유지하고 FX를 생성하지 않습니다."))
	bool bEnabled = false;

	// [v1.6.0] Projectile에 부착해 지속 재생할 Niagara System입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(EditCondition="bEnabled", DisplayName="Niagara 시스템 (NiagaraSystem)", ToolTip="Trail 또는 추진 화염으로 재생할 Niagara System입니다. 비어 있어도 발사체 이동, 충돌과 피해 판정은 유지됩니다."))
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	// [v1.6.0] FX 원점을 Projectile 기준 또는 메시 소켓 우선 방식으로 해석할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(EditCondition="bEnabled", DisplayName="부착 방식 (AttachMode)", ToolTip="Projectile Relative는 CollisionComponent 기준 Transform만 사용합니다. Mesh Socket With Fallback은 메시 소켓을 우선하고 없으면 RelativeTransform을 사용합니다."))
	ECFProjectileFxAttachMode AttachMode = ECFProjectileFxAttachMode::MeshSocketWithFallback;

	// [v1.6.0] Mesh Socket With Fallback 방식에서 우선 검색할 StaticMesh 소켓 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(EditCondition="bEnabled", DisplayName="부착 소켓 이름 (AttachSocketName)", ToolTip="ProjectileStaticMesh에서 우선 사용할 소켓 이름입니다. 소켓이 없으면 오류로 중단하지 않고 RelativeTransform으로 전환합니다."))
	FName AttachSocketName = NAME_None;

				// [v1.7.1] 위치·회전은 Projectile Fallback에 사용하고 Scale은 소켓·Fallback 공통 독립 FX 배율로 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(EditCondition="bEnabled", DisplayName="Fallback 위치·회전 / FX Scale (RelativeTransform)", ToolTip="Location과 Rotation은 Projectile Relative 방식 또는 메시 소켓 누락 시 CollisionComponent 기준 Fallback에만 사용합니다. Scale은 유효한 메시 소켓을 사용할 때도 메시 Scale과 분리된 독립 FX 크기 배율로 항상 적용합니다."))
	FTransform RelativeTransform = FTransform::Identity;
};

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

	// [v1.7.0] 디버그 패널에 표시할 발사체 데이터, 실제 추진, 연속 충돌, 비행 FX와 DamageData 연결 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData", meta=(DisplayName="발사체 요약 생성 (Build Projectile Summary)", ToolTip="디버그 패널과 로그에 표시할 초기 속도, 추진 모터, 연속 충돌, Trail·추진 비행 FX, 스폰 준비와 DamageData 연결 상태를 생성합니다."))
	FString BuildProjectileSummary() const;

		// [v1.1.0] Projectile Actor 스폰 후보 클래스가 지정되어 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData", meta=(DisplayName="발사체 Actor 클래스 지정 여부 (Has Projectile Actor Class)", ToolTip="ProjectileActorClass가 지정되어 실제 Projectile Actor 전환 후보인지 반환합니다. FireMode가 Projectile이어야 실제 스폰 경로로 사용됩니다."))
	bool HasProjectileActorClass() const;

	// [v1.8.0] 안전하게 보정된 Missile Flight 설정을 값으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData|Missile", meta=(DisplayName="유효 미사일 비행 설정 반환 (Get Effective Missile Flight Config)", ToolTip="Clearance 시간·거리, 전환 시간과 Terminal 거리를 안전 범위로 보정한 설정 복사본입니다. 기본값에서는 비활성입니다."))
	FCFMissileFlightConfig GetEffectiveMissileFlightConfig() const;

	// [v1.8.0] 안전하게 보정된 Missile Guidance 설정을 값으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData|Missile", meta=(DisplayName="유효 미사일 유도 설정 반환 (Get Effective Missile Guide Config)", ToolTip="Guidance Mode, 비례항법 계수, 최대 선회율·횡가속도와 Seeker 제한을 안전 범위로 보정한 설정 복사본입니다. 기본값에서는 비활성입니다."))
	FCFMissileGuideConfig GetEffectiveMissileGuideConfig() const;

	// [v1.8.0] Missile Flight·Guidance 활성 상태와 제한값을 한 줄로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileData|Missile", meta=(DisplayName="미사일 Foundation 요약 생성 (Build Missile Foundation Summary)", ToolTip="Flight 사용 여부, 초기 상태, 공격 프로파일, Guidance Mode, 최대 선회율과 횡가속도를 문자열로 반환합니다."))
	FString BuildMissileFoundationSummary() const;

	// [v1.0.0] 발사체 데이터를 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Identity", meta=(DisplayName="발사체 ID (ProjectileId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 발사체를 식별할 이름입니다. 예: ProtoShell"))
	FName ProjectileId = TEXT("ProtoShell");

	// [v1.0.0] 발사체가 생성될 때 사용할 초기 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Movement", meta=(ClampMin="0.0", DisplayName="초기 속도 (InitialSpeed)", ToolTip="Projectile Actor 전환 뒤 발사대에서 분리되는 순간의 초기 속도입니다. 자체 추진 발사체는 이 속도에서 추가 가속을 시작합니다."))
	float InitialSpeed = 6000.0f;

		// [v1.7.0] 비유도 로켓의 점화 지연, 연소 시간, 추진 가속도와 최대 추진 속도 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Propulsion", meta=(DisplayName="추진 모터 설정 (PropulsionConfig)", ToolTip="자체 추진 사용 여부와 점화·연소·가속·최대 속도를 설정합니다. 기본값은 비활성이라 기존 포탄 동작을 유지합니다."))
	FCFProjectilePropulsionConfig PropulsionConfig;

	// [v1.8.0] Released부터 Terminal까지의 미사일 전용 비행 상태 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Missile", meta=(DisplayName="미사일 비행 설정 (MissileFlightConfig)", ToolTip="Clearance, Transition, Attack Profile과 Terminal 설정입니다. bUseMissileFlight 기본값은 False이며 MG-P0-00에서는 런타임에 연결하지 않습니다."))
	FCFMissileFlightConfig MissileFlightConfig;

	// [v1.8.0] 목표 정보 출처와 물리 제한값을 정의하는 미사일 Guidance 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Missile", meta=(DisplayName="미사일 유도 설정 (MissileGuideConfig)", ToolTip="Guidance Mode, Lost Policy, 비례항법 계수, 최대 선회율·횡가속도와 Seeker 제한입니다. bUseGuidance 기본값은 False이며 MG-P0-00에서는 Velocity를 변경하지 않습니다."))
	FCFMissileGuideConfig MissileGuideConfig;

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

	// [v1.9.0] 다른 발사 차량의 Projectile 또는 Hitscan 적중 한 번으로 이 Projectile을 요격 종료할 수 있는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Interception", meta=(DisplayName="요격 가능 (bCanBeIntercepted)", ToolTip="True이면 다른 차량이 발사한 Projectile 또는 Hitscan의 유효 적중 한 번으로 Intercepted 상태로 종료됩니다. 같은 차량 Projectile은 이 값과 무관하게 서로 충돌하지 않습니다."))
	bool bCanBeIntercepted = true;

	// [v1.9.0] Intercepted 종료 시 이 ProjectileData의 기본 Impact FX를 폭발 표현으로 요청할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Interception", meta=(EditCondition="bCanBeIntercepted", DisplayName="요격 시 폭발 FX (bDetonateWhenIntercepted)", ToolTip="True이면 요격 위치에서 DefaultImpactFxData를 재생합니다. False여도 Projectile은 Intercepted로 종료되지만 폭발 FX 요청은 생략합니다."))
	bool bDetonateWhenIntercepted = true;

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

	// [v1.6.0] 발사체 후방에 이어지는 Trail Niagara 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(DisplayName="Trail FX 설정 (TrailFxSettings)", ToolTip="발사체 비행 Trail의 Niagara, 소켓과 Fallback Transform입니다. 기본 소켓은 FX_Trail이며 기본값은 비활성입니다."))
	FCFProjectileAttachedFxSettings TrailFxSettings;

	// [v1.7.0] 실제 모터 Burning 상태에서 사용할 로켓 또는 미사일 추진 화염 Niagara 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FlightFx", meta=(DisplayName="추진 FX 설정 (ThrusterFxSettings)", ToolTip="실제 추진 모터가 Burning 상태일 때 재생할 Niagara, 소켓과 Fallback Transform입니다. 추진 모터가 비활성이면 자산이 연결돼도 재생하지 않습니다."))
	FCFProjectileAttachedFxSettings ThrusterFxSettings;

	// [v1.0.0] 피격 표현 후보를 찾기 위한 임시 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|FX", meta=(DisplayName="기본 Impact FX 데이터", ToolTip="첫 유효 Impact 위치와 표면 노멀에서 재생할 CombatFxData입니다. 비어 있어도 충돌과 피해 판정은 유지합니다."))
	TObjectPtr<UCFCombatFxData> DefaultImpactFxData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Impact", meta=(DisplayName="피격 효과 ID (ImpactEffectId)", ToolTip="후속 VFX / Decal 연결 전 피격 표현 후보를 구분할 임시 ID입니다."))
	FName ImpactEffectId = TEXT("ProtoImpact");

	// [v1.4.0] 이 발사체 또는 가상 발사 데이터가 직접 사용할 기본 DamageData입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Damage", meta=(DisplayName="기본 피해 데이터 (DefaultDamageData)", ToolTip="이 발사체 또는 가상 HitScan / Laser 발사 데이터가 직접 사용할 DamageData입니다. DamageData 직접 참조 슬롯은 ProjectileData에만 둡니다."))
	TObjectPtr<UCFDamageData> DefaultDamageData = nullptr;

	// [v1.4.0] DamageData 미연결 상태를 확인하기 위한 디버그 fallback 피해 프로파일 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Damage", meta=(DisplayName="피해 프로파일 ID (DamageProfileId)", ToolTip="DamageData가 아직 연결되지 않았을 때 디버그 패널에 표시할 피해 프로파일 ID입니다. 실제 피해 계산의 소유권은 DefaultDamageData가 갖습니다."))
	FName DamageProfileId = TEXT("ProtoDirectHit");
};
