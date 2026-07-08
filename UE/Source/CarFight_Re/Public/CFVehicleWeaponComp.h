// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.17.0
// Date: 2026-07-03
// Description: CarFight 차량 전투 장착 프로파일 해석 컴포넌트
// Scope: VehicleData의 MountProfiles, WeaponData, HardpointSlots를 읽어 P0 FireOrigin, 무기 데이터 상태, 터렛 조준 각도를 계산합니다.
// Changelog:
// - v1.17.0: MountProfile legacy 직접 WeaponData / TurretMountData 해석 helper를 제거하고 EquipmentPresetData 전용 경로로 전환.
// - v1.16.0: MountProfile.DefaultEquipmentPresetData를 우선 해석하고 활성 EquipmentPresetData 디버그 상태를 캐시.
// - v1.15.0: 터렛 회전 제한을 MountProfile 각도와의 교집합에서 TurretMountData 단독 기준으로 전환.
// - v1.14.0: 터렛 회전 / 안정화 값의 MountProfile legacy fallback을 런타임에서 제거하고 TurretMountData를 필수 소스로 전환.
// - v1.13.0: DamageData 해석을 ProjectileData 단일 소유 경로로 정리하고 WeaponData DamageData fallback을 제거.
// - v1.12.0: ProjectileData / WeaponData의 DamageData 참조를 해석하고 활성 DamageData 디버그 상태를 캐시.
// - v1.11.0: 터렛 추적 보간 이후 CurrentYaw/Pitch를 최종 유효 각도 안으로 다시 고정.
// - v1.10.0: Pawn 단계에서 Muzzle 소켓으로 보정된 최종 FireOrigin을 마지막 결과로 기록하는 함수를 추가.
// - v1.9.0: HardpointSlot.SocketName이 유효하면 FireOrigin 계산에서 차체 소켓 Transform을 LocalTransform보다 우선 사용.
// - v1.8.0: 활성 터렛 마운트의 Yaw / Pitch 목표각과 회전 추적 상태를 계산하는 런타임 상태를 추가.
// - v1.7.0: 활성 WeaponData의 FireRatePerMinute를 노출하고 런타임 쿨다운 검증은 환산 발사 간격을 사용.
// - v1.6.0: Projectile 실행 요약을 Pool Acquire 기반 실제 실행 경로에 맞게 갱신.
// - v1.5.0: WeaponData.FireMode가 Projectile일 때만 Projectile 스폰 준비 상태로 판정.
// - v1.4.0: 활성 ProjectileData의 ProjectileActorClass 기반 스폰 준비 상태와 전환 요약을 캐시.
// - v1.3.0: 활성 WeaponData의 DefaultProjectileData를 캐시하고 디버그 getter로 노출.
// - v1.2.0: 활성 WeaponData의 MaxRange / CooldownSeconds를 Fire 검증에서 사용할 수 있도록 런타임 getter와 발사 시간 기록을 추가.
// - v1.1.0: 활성 MountProfile의 DefaultWeaponData를 읽고 호환성/요약을 디버그로 노출.
// - v1.0.0: P0 Top_01 터렛 발사 원점 계산을 위한 최소 WeaponComp 추가.
// Migration:
// - DefaultEquipmentPresetData가 지정되면 TurretMountData / WeaponData의 단일 소스로 사용하고, 프리셋 내부 참조가 비면 해당 장비 데이터는 Missing 상태가 된다.
// - 터렛이 조준 목표를 따라가는 중이어도 발사는 막지 않으며, 시각 회전값은 TurretMountData의 Min/Max Yaw/Pitch 안에 고정한다.
// - EquipmentPresetData 내부 TurretMountData가 비어 있으면 터렛 조준 추적은 MissingTurretMountData로 건너뛰며, 발사 / Projectile / 쿨다운 흐름은 유지한다.
// - 기존 AimComp / Fire 함수 시그니처는 유지하고, Pawn에서 WeaponComp FireOrigin을 우선 사용할 수 있게 한다.
// - EquipmentPresetData 내부 WeaponData가 비어 있어도 WeaponRuntime Ready와 FireOrigin 계산은 기존처럼 유지한다.
// - WeaponData가 없거나 호환되지 않으면 MaxRange / FireRatePerMinute는 기존 Aim Profile / 즉시 발사 fallback을 사용한다.
// - ProjectileData 또는 ProjectileActorClass가 비어 있거나 FireMode가 HitScan이면 기존 Dummy HitScan / FireOrigin / 발사 간격 검증 흐름은 유지한다.
// - DamageData 직접 참조는 ProjectileData.DefaultDamageData만 사용하고, HitScan / Laser는 가상 ProjectileData로 연결한다.
// - DamageData가 비어 있어도 발사 가능 여부, Projectile 전환, Dummy HitScan fallback은 변경하지 않는다.
// - 실제 Projectile Actor 확보 / 활성화는 Pawn의 로컬 Fire 적용 단계에서 ProjectilePoolComp를 통해 실행한다.
// - 터렛 각도 상태는 WeaponComp가 소유하고, Pawn은 계산된 각도를 시각 피벗 컴포넌트에 적용만 한다.
// - HardpointSlot.SocketName이 비어 있거나 부모 컴포넌트에 소켓이 없으면 기존 LocalTransform fallback을 유지한다.
// - Muzzle 소켓 FireOrigin 보정은 Pawn이 수행하고, WeaponComp는 최종 결과 기록만 담당한다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleWeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleWeaponComp.generated.h"

class ACFVehiclePawn;
class UCFDamageData;
class UCFEquipmentPresetData;
class UCFProjectileData;
class UCFTurretMountData;
class UCFVehicleData;
class UCFWeaponData;
class USceneComponent;
struct FCFVehicleHardpointSlot;

/**
 * 차량 전투 장착 프로파일을 해석하고 실제 발사 원점을 계산하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleWeaponComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 컴포넌트 값을 초기화합니다.
	UCFVehicleWeaponComp();

	// [v1.0.0] Owner Pawn과 VehicleData 참조를 준비합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(DisplayName="무기 런타임 초기화 (Initialize Weapon Runtime)", ToolTip="Owner 차량 Pawn과 VehicleData를 캐시하고 활성 장착 프로파일을 확인합니다."))
	bool InitializeWeaponRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData);

	// [v1.0.0] 무기 런타임이 활성 장착 프로파일을 사용할 수 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="무기 런타임 준비 여부 (Is Weapon Runtime Ready)", ToolTip="활성 장착 프로파일과 참조 차량 데이터가 준비되었는지 반환합니다."))
	bool IsWeaponRuntimeReady() const { return bWeaponRuntimeReady; }

	// [v1.0.0] 마지막 무기 런타임 초기화 또는 발사 원점 계산 요약을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="무기 런타임 요약 반환 (Get Last Weapon Runtime Summary)", ToolTip="마지막 무기 런타임 초기화 또는 발사 원점 계산 결과 요약을 반환합니다."))
	FString GetLastWeaponRuntimeSummary() const { return LastWeaponRuntimeSummary; }

	// [v1.0.0] 현재 활성 장착 프로파일 ID를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 장착 프로파일 ID 반환 (Get Active Mount Profile Id)", ToolTip="현재 WeaponComp가 우선 사용할 장착 프로파일 ID를 반환합니다."))
	FName GetActiveMountProfileId() const { return ActiveMountProfileId; }

	// [v1.16.0] 현재 활성 장착 프로파일에 연결된 EquipmentPresetData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 EquipmentPresetData 반환 (Get Active Equipment Preset Data)", ToolTip="현재 활성 장착 프로파일에서 우선 해석한 EquipmentPresetData를 반환합니다. 비어 있으면 None입니다."))
	UCFEquipmentPresetData* GetActiveEquipmentPresetData() const { return ActiveEquipmentPresetData; }

	// [v1.16.0] 현재 활성 EquipmentPresetData가 활성 장착 프로파일과 호환되는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 EquipmentPresetData 호환 여부 (Is Active Equipment Preset Compatible)", ToolTip="현재 활성 EquipmentPresetData가 장착 타입과 크기 제한을 통과했는지 반환합니다. EquipmentPresetData가 비어 있으면 False입니다."))
	bool IsActiveEquipmentPresetCompatible() const { return bActiveEquipmentPresetCompatible; }

	// [v1.16.0] 현재 활성 EquipmentPresetData의 디버그 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 EquipmentPresetData 요약 반환 (Get Active Equipment Preset Summary)", ToolTip="현재 활성 EquipmentPresetData의 핵심 조합 요약 문자열을 반환합니다."))
	FString GetActiveEquipmentPresetSummary() const { return ActiveEquipmentPresetSummary; }

	// [v1.8.0] 현재 터렛 조준 추적 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Turret", meta=(DisplayName="터렛 상태 반환 (Get Turret State)", ToolTip="현재 터렛의 목표 Yaw/Pitch와 실제 추적 Yaw/Pitch 상태를 반환합니다."))
	FCFVehicleTurretState GetTurretState() const { return TurretState; }

	// [v1.8.0] 마지막 터렛 조준 추적 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Turret", meta=(DisplayName="터렛 런타임 요약 반환 (Get Last Turret Runtime Summary)", ToolTip="마지막 터렛 조준 추적 계산 결과 요약 문자열을 반환합니다."))
	FString GetLastTurretRuntimeSummary() const { return LastTurretRuntimeSummary; }

	// [v1.1.0] 현재 활성 장착 프로파일에 연결된 WeaponData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 WeaponData 반환 (Get Active Weapon Data)", ToolTip="현재 활성 장착 프로파일에 연결된 WeaponData를 반환합니다. 비어 있으면 None입니다."))
	UCFWeaponData* GetActiveWeaponData() const { return ActiveWeaponData; }

	// [v1.1.0] 현재 활성 WeaponData가 활성 장착 프로파일과 호환되는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 WeaponData 호환 여부 (Is Active Weapon Data Compatible)", ToolTip="현재 활성 WeaponData가 장착 타입과 크기 제한을 통과했는지 반환합니다. WeaponData가 비어 있으면 False입니다."))
	bool IsActiveWeaponDataCompatible() const { return bActiveWeaponDataCompatible; }

	// [v1.1.0] 현재 활성 WeaponData의 디버그 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 WeaponData 요약 반환 (Get Active Weapon Summary)", ToolTip="현재 활성 WeaponData의 핵심 전투 데이터 요약 문자열을 반환합니다."))
	FString GetActiveWeaponSummary() const { return ActiveWeaponSummary; }

	// [v1.3.0] 현재 활성 WeaponData에 연결된 ProjectileData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 ProjectileData 반환 (Get Active Projectile Data)", ToolTip="현재 활성 WeaponData에 연결된 ProjectileData를 반환합니다. 비어 있으면 None입니다."))
	UCFProjectileData* GetActiveProjectileData() const { return ActiveProjectileData; }

	// [v1.3.0] 현재 활성 ProjectileData의 디버그 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 ProjectileData 요약 반환 (Get Active Projectile Summary)", ToolTip="현재 활성 ProjectileData의 핵심 발사체 데이터 요약 문자열을 반환합니다."))
	FString GetActiveProjectileSummary() const { return ActiveProjectileSummary; }

	// [v1.13.0] 현재 활성 ProjectileData에서 해석한 DamageData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 DamageData 반환 (Get Active Damage Data)", ToolTip="현재 활성 ProjectileData에서 해석한 DamageData를 반환합니다. 비어 있으면 None입니다."))
	UCFDamageData* GetActiveDamageData() const { return ActiveDamageData; }

	// [v1.13.0] 현재 활성 DamageData 또는 ProjectileData fallback DamageProfileId를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 피해 ID 반환 (Get Active Damage Id)", ToolTip="현재 활성 DamageData의 DamageId 또는 fallback DamageProfileId를 반환합니다."))
	FName GetActiveDamageId() const { return ActiveDamageId; }

	// [v1.13.0] 현재 활성 DamageData의 디버그 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 DamageData 요약 반환 (Get Active Damage Summary)", ToolTip="현재 활성 DamageData 또는 fallback DamageProfileId의 요약 문자열을 반환합니다."))
	FString GetActiveDamageSummary() const { return ActiveDamageSummary; }

	// [v1.12.0] 현재 활성 DamageData가 어떤 경로로 해석되었는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 DamageData 해석 요약 반환 (Get Active Damage Resolution Summary)", ToolTip="ProjectileData.DefaultDamageData 또는 ProjectileData.DamageProfileId fallback 중 어떤 경로가 사용됐는지 반환합니다."))
	FString GetActiveDamageResolutionSummary() const { return ActiveDamageResolutionSummary; }

	// [v1.6.0] 현재 활성 WeaponData가 Projectile Actor Pool 확보 경로를 사용할 수 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 Projectile 스폰 준비 여부 (Is Active Projectile Spawn Ready)", ToolTip="활성 WeaponData가 Projectile 모드이고 ProjectileData와 ProjectileActorClass가 모두 유효해 Projectile Actor Pool 확보 경로를 사용할 수 있는지 반환합니다."))
	bool IsActiveProjectileSpawnReady() const { return bActiveProjectileSpawnReady; }

	// [v1.4.0] 현재 활성 Projectile 실행 경로 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 Projectile 실행 요약 반환 (Get Active Projectile Execution Summary)", ToolTip="Dummy HitScan 유지 또는 Projectile 전환 준비 상태를 설명하는 요약 문자열을 반환합니다."))
	FString GetActiveProjectileExecutionSummary() const { return ActiveProjectileExecutionSummary; }

	// [v1.2.0] 활성 WeaponData의 최대 사거리를 반환하고, 사용할 수 없으면 fallback 사거리를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 무기 최대 사거리 반환 (Get Active Weapon Max Range)", ToolTip="활성 WeaponData가 유효하고 호환되면 MaxRange를 반환하고, 아니면 입력한 fallback 사거리를 반환합니다."))
	float GetActiveWeaponMaxRange(float FallbackRange) const;

	// [v1.7.0] 활성 WeaponData의 분당 발사속도를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 무기 분당 발사속도 반환 (Get Active Weapon Fire Rate Per Minute)", ToolTip="활성 WeaponData가 유효하고 호환되면 FireRatePerMinute를 반환하고, 아니면 0을 반환합니다. 60이면 1초마다 1발입니다."))
	float GetActiveWeaponFireRatePerMinute() const;

	// [v1.7.0] 활성 WeaponData의 분당 발사속도를 초 단위 발사 간격으로 환산해 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 무기 발사 간격 반환 (Get Active Weapon Fire Interval Seconds)", ToolTip="활성 WeaponData가 유효하고 호환되면 FireRatePerMinute를 초 단위 발사 간격으로 환산해 반환하고, 아니면 0을 반환합니다. 기존 쿨다운 검증 호환용입니다."))
	float GetActiveWeaponCooldownSeconds() const;

	// [v1.2.0] 현재 시간 기준 남은 무기 쿨다운 시간을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="남은 무기 쿨다운 반환 (Get Remaining Cooldown Seconds)", ToolTip="현재 월드 시간 기준 활성 무기의 남은 쿨다운 시간을 반환합니다."))
	float GetRemainingCooldownSeconds(float CurrentTimeSeconds) const;

	// [v1.2.0] 현재 시간 기준 활성 무기가 쿨다운 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="활성 무기 쿨다운 여부 (Is Active Weapon On Cooldown)", ToolTip="현재 월드 시간 기준 활성 무기가 쿨다운 중인지 반환합니다."))
	bool IsActiveWeaponOnCooldown(float CurrentTimeSeconds) const;

	// [v1.2.0] 승인된 발사 시간을 기록해 이후 쿨다운 검증에 사용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(DisplayName="승인 발사 시간 기록 (Record Accepted Fire)", ToolTip="로컬 발사 검증이 승인된 시간을 기록해 활성 무기 쿨다운 계산에 사용합니다."))
	void RecordAcceptedFire(float AcceptedFireTimeSeconds);

	// [v1.2.0] 마지막으로 승인된 발사 시간을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="마지막 승인 발사 시간 반환 (Get Last Accepted Fire Time)", ToolTip="WeaponComp가 마지막으로 기록한 승인 발사 시간입니다. 아직 없으면 음수입니다."))
	float GetLastAcceptedFireTimeSeconds() const { return LastAcceptedFireTimeSeconds; }

	// [v1.0.0] 마지막으로 계산된 발사 원점 결과를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="마지막 발사 원점 반환 (Get Last Fire Origin)", ToolTip="마지막으로 계산된 실제 발사 위치와 방향을 반환합니다."))
	FCFVehicleFireOrigin GetLastFireOrigin() const { return LastFireOrigin; }

	// [v1.0.0] 활성 장착 프로파일과 하드포인트 슬롯에서 실제 발사 원점을 계산합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(DisplayName="발사 원점 계산 (Build Fire Origin)", ToolTip="활성 장착 프로파일과 위치 슬롯을 읽어 실제 발사 위치와 방향을 계산합니다."))
	bool BuildFireOrigin(const FVector& FallbackAimDirection, FCFVehicleFireOrigin& OutFireOrigin);

	// [v1.10.0] Pawn에서 보정한 최종 FireOrigin을 마지막 결과로 기록합니다.
	void RecordResolvedFireOrigin(const FCFVehicleFireOrigin& InFireOrigin, const FString& InRuntimeSummary);

	// [v1.8.0] 활성 터렛 마운트의 Yaw / Pitch 목표각과 현재 추적 각도를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon|Turret", meta=(DisplayName="터렛 조준 상태 갱신 (Update Turret State)", ToolTip="활성 터렛 마운트 데이터와 월드 조준 방향을 기준으로 Yaw/Pitch 목표각과 현재 추적 각도를 갱신합니다."))
	bool UpdateTurretState(float DeltaSeconds, const FVector& WorldAimDirection, const FTransform& TurretReferenceTransform, bool bHasTurretVisual);

	// [v1.8.0] 터렛 조준 추적 상태를 기본값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon|Turret", meta=(DisplayName="터렛 상태 초기화 (Reset Turret State)", ToolTip="터렛 조준 추적 상태와 안정화 누적 시간을 기본값으로 초기화합니다."))
	void ResetTurretState();

private:
	// [v1.0.0] 현재 활성 장착 프로파일을 찾습니다.
	const FCFVehicleMountProfile* FindActiveMountProfile() const;

	// [v1.0.0] 위치 슬롯 ID와 일치하는 하드포인트 슬롯을 찾습니다.
	const FCFVehicleHardpointSlot* FindHardpointSlot(FName LocationSlotId) const;

	// [v1.17.0] 활성 장착 프로파일에 연결된 EquipmentPresetData / WeaponData / ProjectileData / DamageData와 Projectile 스폰 준비 상태를 캐시합니다.
	void CacheActiveWeaponData(const FCFVehicleMountProfile& ActiveMountProfile);

	// [v1.13.0] ProjectileData 단일 소유 기준으로 활성 DamageData를 캐시합니다.
	void CacheActiveDamageData();

	// [v1.0.0] 발사 원점 계산에 사용할 부모 Transform을 반환합니다.
	FTransform ResolveMountParentTransform() const;

	// [v1.0.0] 장착 부모 컴포넌트 이름과 일치하는 SceneComponent를 찾습니다.
	USceneComponent* FindMountParentComponent() const;

	// [v1.0.0] 무기 런타임을 소유한 차량 Pawn입니다.
	UPROPERTY(Transient)
	TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

	// [v1.0.0] 현재 무기 런타임이 읽을 차량 DataAsset입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleData> CachedVehicleData = nullptr;

	// [v1.0.0] 우선 사용할 장착 프로파일 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 장착 프로파일 ID (ActiveMountProfileId)", ToolTip="WeaponComp가 우선 사용할 장착 프로파일 ID입니다. 기본값은 P0 루프 터렛입니다."))
	FName ActiveMountProfileId = TEXT("RoofTurret_MediumOrLarge");

	// [v1.0.0] 하드포인트 LocalTransform을 월드로 바꿀 때 우선 사용할 부모 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="장착 부모 컴포넌트 이름 (MountParentComponentName)", ToolTip="하드포인트 위치를 월드 Transform으로 바꿀 때 우선 기준으로 사용할 컴포넌트 이름입니다. 없으면 차량 Actor Transform을 사용합니다."))
	FName MountParentComponentName = TEXT("SM_Body");

	// [v1.0.0] 현재 WeaponComp가 활성 장착 프로파일을 사용할 수 있는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="무기 런타임 준비 여부 (bWeaponRuntimeReady)", ToolTip="활성 장착 프로파일과 차량 데이터 참조가 준비되었는지 여부입니다."))
	bool bWeaponRuntimeReady = false;

	// [v1.16.0] 활성 장착 프로파일에서 우선 해석한 EquipmentPresetData입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 EquipmentPresetData (ActiveEquipmentPresetData)", ToolTip="현재 활성 장착 프로파일에서 우선 해석한 EquipmentPresetData입니다."))
	TObjectPtr<UCFEquipmentPresetData> ActiveEquipmentPresetData = nullptr;

	// [v1.16.0] 활성 EquipmentPresetData가 활성 장착 프로파일과 호환되는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 EquipmentPresetData 호환 여부 (bActiveEquipmentPresetCompatible)", ToolTip="현재 활성 EquipmentPresetData가 장착 타입과 크기 제한을 통과했는지 여부입니다."))
	bool bActiveEquipmentPresetCompatible = false;

	// [v1.16.0] 활성 EquipmentPresetData 핵심 값 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 EquipmentPresetData 요약 (ActiveEquipmentPresetSummary)", ToolTip="현재 활성 EquipmentPresetData의 핵심 조합 요약 문자열입니다."))
	FString ActiveEquipmentPresetSummary = TEXT("EquipmentPresetData: MissingOptional");

	// [v1.1.0] 활성 장착 프로파일에 연결된 WeaponData입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 WeaponData (ActiveWeaponData)", ToolTip="현재 활성 장착 프로파일에 연결된 WeaponData입니다."))
	TObjectPtr<UCFWeaponData> ActiveWeaponData = nullptr;

	// [v1.1.0] 활성 WeaponData가 활성 장착 프로파일과 호환되는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 WeaponData 호환 여부 (bActiveWeaponDataCompatible)", ToolTip="현재 활성 WeaponData가 장착 타입과 크기 제한을 통과했는지 여부입니다."))
	bool bActiveWeaponDataCompatible = false;

	// [v1.1.0] 활성 WeaponData 핵심 값 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 WeaponData 요약 (ActiveWeaponSummary)", ToolTip="현재 활성 WeaponData의 핵심 전투 데이터 요약 문자열입니다."))
	FString ActiveWeaponSummary = TEXT("WeaponData: MissingOptional");

	// [v1.3.0] 활성 WeaponData에 연결된 ProjectileData입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 ProjectileData (ActiveProjectileData)", ToolTip="현재 활성 WeaponData에 연결된 ProjectileData입니다."))
	TObjectPtr<UCFProjectileData> ActiveProjectileData = nullptr;

	// [v1.3.0] 활성 ProjectileData 핵심 값 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 ProjectileData 요약 (ActiveProjectileSummary)", ToolTip="현재 활성 ProjectileData의 핵심 발사체 데이터 요약 문자열입니다."))
	FString ActiveProjectileSummary = TEXT("ProjectileData: MissingOptional");

	// [v1.13.0] 활성 ProjectileData에서 해석한 DamageData입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 DamageData (ActiveDamageData)", ToolTip="현재 활성 ProjectileData에서 해석한 DamageData입니다."))
	TObjectPtr<UCFDamageData> ActiveDamageData = nullptr;

	// [v1.13.0] 활성 DamageData의 DamageId 또는 ProjectileData fallback DamageProfileId입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 피해 ID (ActiveDamageId)", ToolTip="현재 활성 DamageData의 DamageId 또는 fallback DamageProfileId입니다."))
	FName ActiveDamageId = NAME_None;

	// [v1.13.0] 활성 DamageData 핵심 값 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 DamageData 요약 (ActiveDamageSummary)", ToolTip="현재 활성 DamageData 또는 fallback DamageProfileId의 요약 문자열입니다."))
	FString ActiveDamageSummary = TEXT("DamageData: MissingOptional");

	// [v1.12.0] 활성 DamageData가 어떤 경로로 해석되었는지 설명하는 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 DamageData 해석 요약 (ActiveDamageResolutionSummary)", ToolTip="ProjectileData.DefaultDamageData 또는 ProjectileData.DamageProfileId fallback 중 어떤 경로가 사용됐는지 설명합니다."))
	FString ActiveDamageResolutionSummary = TEXT("DamageResolution: MissingOptional");

	// [v1.5.0] 활성 WeaponData가 Projectile Actor 스폰 경로를 사용할 수 있는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 Projectile 스폰 준비 여부 (bActiveProjectileSpawnReady)", ToolTip="활성 WeaponData가 Projectile 모드이고 ProjectileData와 ProjectileActorClass가 모두 유효한지 여부입니다."))
	bool bActiveProjectileSpawnReady = false;

	// [v1.4.0] 활성 Projectile 실행 경로를 설명하는 요약 문자열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 Projectile 실행 요약 (ActiveProjectileExecutionSummary)", ToolTip="Dummy HitScan 유지 또는 Projectile 전환 준비 상태를 설명하는 요약 문자열입니다."))
	FString ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: DummyHitScanFallback");

	// [v1.2.0] 마지막으로 승인된 발사 시간입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="마지막 승인 발사 시간 (LastAcceptedFireTimeSeconds)", ToolTip="WeaponComp가 마지막으로 기록한 승인 발사 시간입니다. 아직 없으면 음수입니다."))
	float LastAcceptedFireTimeSeconds = -1.0f;

	// [v1.0.0] 마지막 무기 런타임 처리 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="무기 런타임 요약 (LastWeaponRuntimeSummary)", ToolTip="마지막 무기 런타임 초기화 또는 발사 원점 계산 결과 요약입니다."))
	FString LastWeaponRuntimeSummary = TEXT("NotInitialized");

	// [v1.8.0] 현재 터렛 조준 추적 상태입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 상태 (TurretState)", ToolTip="현재 터렛의 목표 Yaw/Pitch와 실제 추적 Yaw/Pitch 상태입니다."))
	FCFVehicleTurretState TurretState;

	// [v1.8.0] 터렛이 목표 각도 허용 오차 안에 머문 누적 시간입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 안정화 누적 시간 (TurretSettleElapsedSeconds)", ToolTip="터렛이 목표 각도 허용 오차 안에 머문 누적 시간입니다."))
	float TurretSettleElapsedSeconds = 0.0f;

	// [v1.8.0] 마지막 터렛 조준 추적 계산 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 런타임 요약 (LastTurretRuntimeSummary)", ToolTip="마지막 터렛 조준 추적 계산 결과 요약입니다."))
	FString LastTurretRuntimeSummary = TEXT("TurretRuntime: NotInitialized");

	// [v1.0.0] 마지막으로 계산한 발사 원점입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="마지막 발사 원점 (LastFireOrigin)", ToolTip="마지막으로 계산한 실제 발사 위치와 방향입니다."))
	FCFVehicleFireOrigin LastFireOrigin;
};
