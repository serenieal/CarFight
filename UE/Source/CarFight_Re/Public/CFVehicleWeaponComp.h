// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.25.0
// Date: 2026-08-19
// Description: CarFight 차량 전투 장착 프로파일·Player-facing Weapon Selection·무기 Charge·Heat와 선택 대상 사용 평가 컴포넌트
// Scope: Applied Fitting의 실제 weapon-bearing 고정 순서를 선택 Runtime으로 보존하고 FireOrigin, Ammo identity, per-weapon Cooldown·Charge·Heat와 활성 WeaponData를 안전하게 전환합니다.
// Changelog:
// - v1.25.0: UI-P0-06 explicit WeaponCharge를 선택 순번별 독립 Runtime으로 소유하고 자연 회복·승인 한 발 소비·충전 부족 판정·HUD read API를 추가. VehicleBattery와 독립 유지.
// - v1.24.0: UI-P0-06 Applied Fitting 기반 Weapon Selection Runtime, SelectedWeaponIndex와 Player-facing DisplayName read API, 무기별 Cooldown·Heat 상태 보존/비선택 Heat 냉각을 추가. 내부 MountProfileId는 UI 의미로 노출하지 않음.
// - v1.23.0: UI-P0-06 활성 WeaponData의 explicit Heat 설정을 per-weapon Runtime으로 소유하고 자연 냉각·과열·승인 발사 누적 public read API를 추가.
// - v1.22.0: FIT-P0-04 Snapshot EquipmentPresetData Override와 활성 프로파일 명시 초기화 API를 추가.
// - v1.21.0: 활성 WeaponData의 안전한 런처 Release 설정과 전용 요약 Getter를 추가.
// - v1.20.0: 활성 WeaponData의 안전한 런처 발사 패턴 설정과 전용 요약 Getter를 추가.
// - v1.19.0: 활성 TurretMountData 캐시와 성공 발사 기반 SingleCycle Muzzle 순환·Reset·Debug 계약을 추가.
// - v1.18.1: 이동 중 사거리 진입·이탈을 반영하도록 저빈도 선택 대상 재평가 Tick을 추가.
// - v1.18.0: TS-P0-07 TargetSelectComp 구독, 활성 무기 대상 평가 요청·캐시·변경 이벤트를 추가.
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
// - v1.25.0 WeaponCharge는 활성·호환 WeaponData의 Maximum/Initial/PerShot/Recovery explicit 값이 유효할 때만 켜진다. 기존 all-zero Asset은 Disabled이며 발사를 제한하지 않고 VehicleBattery fallback도 사용하지 않는다.
// - v1.24.0 Snapshot Weapon Selection은 Applied Fitting이 전달한 고정 순서만 사용하고 선택 항목마다 Cooldown/Heat를 독립 보존한다. Legacy·single snapshot 초기화는 선택 Runtime을 비워 기존 단일 무기 동작을 유지한다.
// - v1.23.0 Heat Runtime은 활성·호환 WeaponData의 HeatPerShot/MaxHeat/HeatDissipationPerSecond가 모두 명시됐을 때만 켜진다. 무기/출격 재초기화 시 Heat는 0으로 reset하고 동일 WeaponData의 단순 FireOrigin 재해석은 현재 Heat를 보존한다.
// - 활성 WeaponData가 없거나 호환되지 않으면 런처 발사 패턴 Getter는 기존 단발 동작과 같은 SingleCycle / 1발 기본값을 반환한다.
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
#include "CFTargetUseTypes.h"
#include "CFLauncherTypes.h"
#include "CFWeaponChargeRuntime.h"
#include "CFWeaponHeatRuntime.h"
#include "CFWeaponSelectTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleWeaponComp.generated.h"

class ACFVehiclePawn;
class UCFDamageData;
class UCFEquipmentPresetData;
class UCFProjectileData;
class UCFTargetSelectComp;
class UCFTurretMountData;
class UCFVehicleData;
class UCFWeaponData;
class USceneComponent;
struct FCFVehicleHardpointSlot;

// [v1.18.0] 활성 무기의 선택 대상 사용 가능 결과가 변경됐을 때 전달하는 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFActiveWeaponTargetUseChangedSignature, FCFTargetUseResult, TargetUseResult);

/**
 * 차량 전투 장착 프로파일을 해석하고 실제 발사 원점과 선택 대상 사용 가능 상태를 계산하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleWeaponComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 컴포넌트 값을 초기화합니다.
UCFVehicleWeaponComp();

// [v1.18.0] 선택 컴포넌트 이벤트 구독을 정리합니다.
virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

// [v1.18.1] 이동 중 대상 거리 변화를 낮은 빈도로 재평가합니다.
virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.0.0] Owner Pawn과 VehicleData 참조를 준비합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(DisplayName="무기 런타임 초기화 (Initialize Weapon Runtime)", ToolTip="Owner 차량 Pawn과 VehicleData를 캐시하고 활성 장착 프로파일을 확인합니다."))
	bool InitializeWeaponRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData);

		// [v1.22.0] 지정 활성 프로파일에서 VehicleData 기본 장비를 사용하는 Legacy Runtime을 초기화합니다.
	bool InitializeWeaponRuntimeForActiveProfile(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData, FName InActiveMountProfileId);

		// [v1.22.0] 지정 활성 프로파일에 Snapshot 장비 또는 빈 장착을 적용해 Runtime을 초기화합니다.
	bool InitializeWeaponRuntimeFromFitting(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData, FName InActiveMountProfileId, UCFEquipmentPresetData* InEquipmentPresetData);

	// [v1.24.0] Applied Fitting의 실제 weapon-bearing 고정 순서와 선택 인덱스로 다중 무기 Runtime을 초기화합니다.
	bool InitializeWeaponSelectionRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData, const TArray<FCFWeaponSelectRuntimeItem>& InSelectableWeapons, int32 InSelectedWeaponIndex);

	// [v1.24.0] Applied Fitting 기반 실제 Weapon Selection Runtime이 활성화됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Selection", meta=(DisplayName="무기 선택 Runtime 사용 여부", ToolTip="Applied Fitting의 실제 weapon-bearing mount 목록이 Weapon Selection Runtime으로 초기화됐는지 반환합니다. 내부 MountProfileId는 UI에 노출하지 않습니다."))
	bool HasWeaponSelectionRuntime() const { return !SelectableWeapons.IsEmpty() && SelectableWeapons.IsValidIndex(SelectedWeaponIndex); }

	// [v1.24.0] Player-facing 고정 표시 순서에 포함된 실제 선택 가능 무기 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Selection", meta=(DisplayName="선택 가능 무기 수 반환", ToolTip="Applied Fitting의 weapon-bearing ResolvedMounts 고정 순서에 포함된 실제 선택 가능 무기 수를 반환합니다."))
	int32 GetSelectableWeaponCount() const { return SelectableWeapons.Num(); }

	// [v1.24.0] Player-facing 고정 표시 순서에서 현재 활성 무기의 0-based 선택 인덱스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Selection", meta=(DisplayName="현재 선택 무기 인덱스 반환", ToolTip="Applied Fitting의 고정 표시 순서에서 현재 선택된 무기의 0-based 인덱스를 반환합니다. 선택 Runtime이 없으면 -1입니다."))
	int32 GetSelectedWeaponIndex() const { return HasWeaponSelectionRuntime() ? SelectedWeaponIndex : INDEX_NONE; }

	// [v1.24.0] 지정 선택 인덱스에 실제 Player-facing DisplayName source가 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Selection", meta=(DisplayName="선택 무기 표시 이름 사용 가능 여부", ToolTip="지정 순번의 실제 EquipmentPresetData에 비어 있지 않은 DisplayName이 있는지 반환합니다. 내부 ID나 Asset 이름 fallback은 사용하지 않습니다."))
	bool IsSelectableWeaponDisplayNameAvailable(int32 WeaponIndex) const;

	// [v1.24.0] 지정 선택 인덱스의 실제 EquipmentPresetData.DisplayName을 반환하고 없으면 빈 Text를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Selection", meta=(DisplayName="선택 무기 표시 이름 반환", ToolTip="지정 순번의 실제 EquipmentPresetData.DisplayName만 반환합니다. WeaponId, EquipmentId, MountProfileId, Asset 이름으로 fallback하지 않습니다."))
	FText GetSelectableWeaponDisplayName(int32 WeaponIndex) const;

	// [v1.22.0] 현재 Runtime이 Snapshot 장비 선택을 사용하는지 반환합니다.
	bool IsUsingRuntimeEquipmentPresetOverride() const { return bUseRuntimeEquipmentPresetOverride; }

	// [v1.22.0] 현재 Snapshot EquipmentPresetData를 반환합니다. 빈 장착이면 None입니다.
	UCFEquipmentPresetData* GetRuntimeEquipmentPresetOverride() const { return RuntimeEquipmentPresetOverride; }

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

	// [v1.20.0] 활성 WeaponData의 안전하게 보정된 런처 발사 패턴 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="활성 런처 발사 패턴 반환 (Get Active Launcher Fire Pattern Config)", ToolTip="활성 WeaponData가 유효하면 보정된 SingleCycle·Ripple·Salvo 설정을 반환하고, 없거나 호환되지 않으면 SingleCycle 1발 기본값을 반환합니다."))
	FCFLauncherFirePatternConfig GetActiveLauncherFirePatternConfig() const;

		// [v1.20.0] 활성 WeaponData의 런처 발사 패턴 설정을 한 줄로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="활성 런처 발사 패턴 요약 반환 (Get Active Launcher Fire Pattern Summary)", ToolTip="활성 WeaponData의 발사 패턴, 유효 발사 수, Ripple 간격, Salvo 동시 처리와 실패·쿨다운 정책을 문자열로 반환합니다."))
	FString GetActiveLauncherFirePatternSummary() const;

	// [v1.21.0] 활성 WeaponData의 안전하게 보정된 런처 Release 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="활성 런처 Release 설정 반환 (Get Active Launcher Release Config)", ToolTip="활성 WeaponData가 유효하면 Direct·Angled·Vertical 설정을 반환하고, 없거나 호환되지 않으면 기존 직사와 같은 Direct / 차량 속도 상속 0을 반환합니다."))
	FCFLauncherReleaseConfig GetActiveLauncherReleaseConfig() const;

	// [v1.21.0] 활성 WeaponData의 런처 Release 설정을 한 줄로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="활성 런처 Release 요약 반환 (Get Active Launcher Release Summary)", ToolTip="Release Mode, 로컬 사출 방향, 사출 속력, 차량 속도 상속 비율과 안전 검사 거리를 문자열로 반환합니다."))
	FString GetActiveLauncherReleaseSummary() const;

// [v1.18.0] 활성 무기 데이터와 현재 런타임 상태를 장비 타겟 사용 요청으로 변환합니다.
UFUNCTION(BlueprintPure, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="활성 무기 타겟 사용 요청 생성", ToolTip="활성 WeaponData의 ID, MaxRange와 TargetUsePolicy를 사용해 선택 대상 평가 요청을 생성합니다."))
FCFTargetUseRequest BuildActiveWeaponTargetUseRequest() const;

// [v1.18.0] 현재 선택 대상이 활성 무기에 사용 가능한지 즉시 평가합니다.
UFUNCTION(BlueprintPure, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="활성 무기 선택 타겟 평가", ToolTip="TargetSelectComp의 현재 선택 상태를 읽기 전용으로 평가하며 조준 또는 발사 방향은 변경하지 않습니다."))
FCFTargetUseResult EvaluateSelectedTargetForActiveWeapon() const;

// [v1.18.0] 활성 무기의 선택 대상 사용 결과 캐시를 갱신합니다.
UFUNCTION(BlueprintCallable, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="활성 무기 타겟 사용 결과 갱신", ToolTip="현재 선택 상태를 다시 평가하고 실제 결과 변경 때 이벤트를 발생시킵니다."))
bool RefreshActiveWeaponTargetUseResult();

// [v1.18.0] 마지막으로 캐시된 활성 무기 대상 사용 결과를 반환합니다.
UFUNCTION(BlueprintPure, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="활성 무기 타겟 사용 결과 반환"))
FCFTargetUseResult GetLastActiveWeaponTargetUseResult() const { return LastActiveWeaponTargetUseResult; }

// [v1.18.0] 마지막 활성 무기 대상 사용 평가의 한 줄 요약을 반환합니다.
UFUNCTION(BlueprintPure, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="활성 무기 타겟 사용 요약 반환"))
FString GetLastActiveWeaponTargetUseSummary() const { return LastActiveWeaponTargetUseSummary; }

// [v1.18.0] 활성 무기의 선택 대상 사용 결과가 실제로 변경될 때 호출됩니다.
UPROPERTY(BlueprintAssignable, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="활성 무기 타겟 사용 결과 변경 이벤트"))
FCFActiveWeaponTargetUseChangedSignature OnActiveWeaponTargetUseChanged;

// [v1.18.1] 선택 대상의 이동과 사거리 변화를 자동 재평가할지 여부입니다.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Weapon|TargetUse", meta=(DisplayName="타겟 사용 결과 자동 갱신", ToolTip="True이면 선택 대상이 있는 동안 일정 간격으로 거리와 사용 가능 상태를 다시 평가합니다."))
bool bAutoRefreshTargetUseResult = true;

// [v1.18.1] 선택 대상 거리와 사용 가능 상태를 다시 평가하는 간격입니다.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Weapon|TargetUse", meta=(ClampMin="0.01", Units="s", DisplayName="타겟 사용 결과 갱신 간격", ToolTip="선택 대상 이동에 따른 사거리 진입과 이탈을 다시 평가하는 초 단위 간격입니다."))
float TargetUseRefreshIntervalSeconds = 0.10f;

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

			// [v1.25.0] 활성 WeaponData에 실제 Charge Runtime이 구성됐는지 반환하며 Weapon Selection에서는 선택 항목의 독립 상태를 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Charge", meta=(DisplayName="활성 무기 Charge Runtime 사용 여부", ToolTip="현재 활성·호환 WeaponData의 실제 내부 Charge Runtime 활성 여부를 반환합니다. VehicleBattery와는 별개입니다."))
	bool IsActiveWeaponChargeRuntimeEnabled() const;

	// [v1.25.0] 현재 활성 무기의 실제 내부 Charge를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Charge", meta=(DisplayName="현재 무기 Charge 반환", ToolTip="현재 활성 무기의 실제 내부 Charge를 반환합니다. 다중 무기 선택에서는 선택된 무기의 독립 Charge 상태입니다."))
	float GetCurrentWeaponCharge() const;

	// [v1.25.0] 현재 활성 무기의 명시된 최대 내부 Charge를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Charge", meta=(DisplayName="활성 무기 최대 Charge 반환", ToolTip="현재 활성 Charge Runtime의 최대 내부 충전량을 반환합니다. Charge Runtime이 비활성이면 0입니다."))
	float GetActiveWeaponMaximumCharge() const;

	// [v1.25.0] 현재 내부 Charge를 최대값 기준 0~1 비율로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Charge", meta=(DisplayName="활성 무기 Charge 비율 반환", ToolTip="현재 활성 무기의 실제 내부 Charge를 최대 Charge 기준 0부터 1까지의 비율로 반환합니다."))
	float GetActiveWeaponChargeRatio() const;

	// [v1.25.0] 현재 Charge 상태에서 실제 한 발을 추가로 승인할 수 있는지 반환합니다.
	bool CanActiveWeaponAcceptChargeShot() const;

	// [v1.25.0] 실제 승인된 한 발의 Charge를 현재 선택 무기의 독립 Runtime에서 정확히 한 번 소비합니다.
	void RecordAcceptedWeaponShotCharge();

		// [v1.24.0] 활성 WeaponData에 실제 Heat Runtime이 구성됐는지 반환하며 Weapon Selection에서는 선택 항목의 독립 상태를 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Heat", meta=(DisplayName="활성 무기 Heat Runtime 사용 여부", ToolTip="현재 활성·호환 WeaponData의 실제 Heat Runtime 활성 여부를 반환합니다. 다중 무기 선택에서는 선택된 무기의 독립 Heat 상태만 읽습니다."))
	bool IsActiveWeaponHeatRuntimeEnabled() const;

	// [v1.24.0] 현재 활성 무기에 누적된 실제 Heat를 반환하며 Weapon Selection에서는 선택 항목의 독립 상태를 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Heat", meta=(DisplayName="현재 무기 Heat 반환", ToolTip="현재 활성 무기에 실제 누적된 Heat를 반환합니다. 다중 무기 선택에서는 선택된 무기의 독립 Heat 상태입니다."))
	float GetCurrentWeaponHeat() const;

	// [v1.24.0] 현재 활성 무기의 명시된 최대 Heat를 반환하며 Weapon Selection에서는 선택 항목의 독립 상태를 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Heat", meta=(DisplayName="활성 무기 최대 Heat 반환", ToolTip="현재 활성 Heat Runtime의 최대 열량을 반환합니다. Heat Runtime이 비활성이면 0입니다."))
	float GetActiveWeaponMaximumHeat() const;

	// [v1.24.0] 현재 Heat를 최대 Heat 기준 0~1 비율로 반환하며 Weapon Selection에서는 선택 항목의 독립 상태를 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Heat", meta=(DisplayName="활성 무기 Heat 비율 반환", ToolTip="현재 활성 무기의 실제 Heat를 최대 Heat 기준 0부터 1까지의 비율로 반환합니다."))
	float GetActiveWeaponHeatRatio() const;

	// [v1.24.0] 현재 활성 무기가 MaxHeat 도달 후 냉각 대기 중인지 반환하며 Weapon Selection에서는 선택 항목의 독립 상태를 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Heat", meta=(DisplayName="활성 무기 과열 여부", ToolTip="현재 활성 무기가 최대 Heat에 도달해 다음 표준 한 발을 위한 여유가 생길 때까지 발사가 차단되는지 반환합니다."))
	bool IsActiveWeaponOverheated() const;

	// [v1.24.0] 현재 Heat 상태에서 실제 한 발을 추가로 승인할 수 있는지 반환합니다.
	bool CanActiveWeaponAcceptHeatShot() const;

	// [v1.24.0] 실제 승인된 한 발의 Heat를 현재 선택 무기의 독립 Runtime에 정확히 한 번 누적합니다.
	void RecordAcceptedWeaponShotHeat();

	// [v1.24.0] 마지막으로 승인된 발사 시간을 반환하며 Weapon Selection에서는 현재 선택 무기의 독립 시간을 사용합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(DisplayName="마지막 승인 발사 시간 반환 (Get Last Accepted Fire Time)", ToolTip="현재 활성 무기의 마지막 승인 발사 시간입니다. 다중 무기 선택에서는 선택된 무기의 독립 쿨다운 상태를 반환합니다."))
	float GetLastAcceptedFireTimeSeconds() const;

	// [v1.19.0] 현재 EquipmentPresetData에서 해석한 활성 TurretMountData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="활성 TurretMountData 반환 (Get Active Turret Mount Data)", ToolTip="현재 활성 장비 프리셋에서 해석한 TurretMountData입니다. Muzzle 배열과 발사 정책의 원본입니다."))
		UCFTurretMountData* GetActiveTurretMountData() const { return CachedActiveTurretMountData; }

	// [v1.19.0] 다음 FireOrigin 해결이 검색을 시작할 MuzzleSocketNames 배열 인덱스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="다음 Muzzle 인덱스 반환 (Get Next Muzzle Socket Index)", ToolTip="다음 SingleCycle 발사 원점 계산이 검색을 시작할 MuzzleSocketNames 배열 인덱스입니다."))
	int32 GetNextMuzzleSocketIndex() const { return NextMuzzleSocketIndex; }

	// [v1.19.0] Pawn이 실제 메쉬에서 해결한 현재 Muzzle 선택을 Debug 상태로 기록합니다.
	void RecordResolvedMuzzleSelection(FName MuzzleSocketName, int32 MuzzleSocketIndex, int32 MuzzleSocketCount);

	// [v1.19.0] 승인된 발사에 사용된 Muzzle 다음 인덱스로 SingleCycle 상태를 진행합니다.
	void AdvanceMuzzleSequenceAfterAcceptedFire(FName MuzzleSocketName, int32 MuzzleSocketIndex, int32 MuzzleSocketCount);

	// [v1.19.0] SingleCycle Muzzle 인덱스와 마지막 해결·발사 상태를 기본값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon|Launcher", meta=(DisplayName="Muzzle 순서 초기화 (Reset Muzzle Sequence)", ToolTip="다음 Muzzle 인덱스를 0으로 되돌리고 마지막 해결·발사 Muzzle 상태를 초기화합니다."))
	void ResetMuzzleSequence();

	// [v1.19.0] 현재 SingleCycle Muzzle 순서와 마지막 해결·발사 결과를 한 줄로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon|Launcher", meta=(DisplayName="Muzzle 순서 요약 생성 (Build Muzzle Sequence Summary)", ToolTip="다음 Muzzle 인덱스, 마지막 해결·발사 Muzzle와 승인 발사 기반 진행 횟수를 표시합니다."))
	FString BuildMuzzleSequenceSummary() const;

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
	friend class ACFVehiclePawn;

	// [v1.22.0] Legacy 또는 Snapshot Override 설정을 유지한 채 공통 Runtime 초기화를 수행합니다.
	bool InitializeWeaponRuntimeInternal(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData);

	// [v1.24.0] 이미 초기화된 Weapon Selection Runtime에서 새 고정 순번을 활성화하고 실패하면 이전 선택을 복원합니다.
	bool ApplySelectedWeaponIndex(int32 NewWeaponIndex);

		// [v1.25.0] Legacy 또는 single-snapshot 초기화 전 다중 무기 선택 상태와 per-weapon Cooldown·Charge·Heat를 비웁니다.
	void ClearWeaponSelectionRuntime();

	// [v1.25.0] 현재 선택 인덱스의 per-weapon Charge Runtime을 반환하고 선택 Runtime이 없으면 nullptr를 반환합니다.
	FCFWeaponChargeRuntime* GetSelectedWeaponChargeRuntime();

	// [v1.25.0] 현재 선택 인덱스의 per-weapon Charge Runtime을 const로 반환하고 선택 Runtime이 없으면 nullptr를 반환합니다.
	const FCFWeaponChargeRuntime* GetSelectedWeaponChargeRuntime() const;

	// [v1.24.0] 현재 선택 인덱스의 per-weapon Heat Runtime을 반환하고 선택 Runtime이 없으면 nullptr를 반환합니다.
	FCFWeaponHeatRuntime* GetSelectedWeaponHeatRuntime();

	// [v1.24.0] 현재 선택 인덱스의 per-weapon Heat Runtime을 const로 반환하고 선택 Runtime이 없으면 nullptr를 반환합니다.
	const FCFWeaponHeatRuntime* GetSelectedWeaponHeatRuntime() const;

	// [v1.22.0] VehicleData 기본값 또는 Snapshot Override 장비를 해석합니다.
	UCFEquipmentPresetData* ResolveActiveEquipmentPresetData(const FCFVehicleMountProfile& ActiveMountProfile) const;

	// [v1.0.0] 현재 활성 장착 프로파일을 찾습니다.
					void BindTargetSelectEvents();
	void UnbindTargetSelectEvents();

					UFUNCTION()
	void HandleSelectedTargetChangedForWeapon(AActor* PreviousTarget, AActor* NewTarget, FCFTargetDisplayInfo DisplayInfo);

	UFUNCTION()
	void HandleSelectedTargetClearedForWeapon(AActor* ClearedTarget, ECFTargetClearReason ClearReason);

	UFUNCTION()
	void HandleSelectedTargetValidityChangedForWeapon(AActor* TargetActor, bool bIsValidTarget);

	UFUNCTION()
	void HandleSelectedTargetTrackStateChangedForWeapon(AActor* TargetActor, ECFTargetTrackState PreviousState, ECFTargetTrackState NewState);

	const FCFVehicleMountProfile* FindActiveMountProfile() const;

	// [v1.0.0] 위치 슬롯 ID와 일치하는 하드포인트 슬롯을 찾습니다.
	const FCFVehicleHardpointSlot* FindHardpointSlot(FName LocationSlotId) const;

		// [v1.17.0] 활성 장착 프로파일에 연결된 EquipmentPresetData / WeaponData / ProjectileData / DamageData와 Projectile 스폰 준비 상태를 캐시합니다.
	void CacheActiveWeaponData(const FCFVehicleMountProfile& ActiveMountProfile);

		// [v1.25.0] 현재 활성 WeaponData의 explicit Charge 설정을 Runtime에 반영하되 같은 Source/설정이면 현재 Charge를 보존합니다.
	void RefreshActiveWeaponChargeRuntimeConfig();

	// [v1.23.0] 현재 활성 WeaponData의 explicit Heat 설정을 Runtime에 반영하되 같은 Source/설정이면 누적 Heat를 보존합니다.
	void RefreshActiveWeaponHeatRuntimeConfig();

	// [v1.13.0] ProjectileData 단일 소유 기준으로 활성 DamageData를 캐시합니다.
	void CacheActiveDamageData();

	// [v1.0.0] 발사 원점 계산에 사용할 부모 Transform을 반환합니다.
	FTransform ResolveMountParentTransform() const;

	// [v1.0.0] 장착 부모 컴포넌트 이름과 일치하는 SceneComponent를 찾습니다.
	USceneComponent* FindMountParentComponent() const;

	// [v1.0.0] 무기 런타임을 소유한 차량 Pawn입니다.
	UPROPERTY(Transient)
					TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

	// [v1.18.0] 활성 무기 대상 평가 갱신을 위해 구독 중인 TargetSelectComp입니다.
	TWeakObjectPtr<UCFTargetSelectComp> BoundTargetSelectComp;

	// [v1.18.0] 외부 UI와 장비 피드백이 읽을 마지막 대상 사용 평가 결과입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|TargetUse", meta=(AllowPrivateAccess="true", DisplayName="마지막 활성 무기 타겟 사용 결과"))
	FCFTargetUseResult LastActiveWeaponTargetUseResult;

	// [v1.18.0] 마지막 대상 사용 평가를 한 줄로 표현한 디버그 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|TargetUse", meta=(AllowPrivateAccess="true", DisplayName="마지막 활성 무기 타겟 사용 요약"))
				FString LastActiveWeaponTargetUseSummary = TEXT("ActiveWeaponTargetUse: NotInitialized");

			float TargetUseRefreshElapsedSeconds = 0.0f;

	// [v1.0.0] 현재 무기 런타임이 읽을 차량 DataAsset입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleData> CachedVehicleData = nullptr;

		// [v1.0.0] 우선 사용할 장착 프로파일 ID입니다. Weapon Selection에서는 내부 FireOrigin·Ammo identity로만 사용하고 Player-facing 그룹명으로 노출하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 장착 프로파일 ID (ActiveMountProfileId)", ToolTip="WeaponComp 내부에서 활성 mount를 찾는 ID입니다. Player-facing 무기 이름이나 그룹으로 표시하지 않습니다."))
	FName ActiveMountProfileId = TEXT("RoofTurret_MediumOrLarge");

	// [v1.24.0] Applied Fitting의 weapon-bearing ResolvedMounts를 고정 순서로 보존한 실제 선택 가능 무기 목록입니다.
	UPROPERTY(Transient)
	TArray<FCFWeaponSelectRuntimeItem> SelectableWeapons;

	// [v1.24.0] SelectableWeapons에서 현재 활성 무기의 0-based 선택 인덱스입니다.
	int32 SelectedWeaponIndex = INDEX_NONE;

	// [v1.24.0] 선택 가능 무기마다 독립 보존하는 마지막 승인 발사 시각입니다.
	TArray<float> SelectableWeaponLastAcceptedFireTimes;

		// [v1.25.0] 선택 가능 무기마다 독립 보존하고 비선택 상태에서도 자연 회복하는 Charge Runtime입니다.
	TArray<FCFWeaponChargeRuntime> SelectableWeaponChargeRuntimes;

	// [v1.24.0] 선택 가능 무기마다 독립 보존하고 비선택 상태에서도 자연 냉각하는 Heat Runtime입니다.
	TArray<FCFWeaponHeatRuntime> SelectableWeaponHeatRuntimes;

	// [v1.0.0] 하드포인트 LocalTransform을 월드로 바꿀 때 우선 사용할 부모 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="장착 부모 컴포넌트 이름 (MountParentComponentName)", ToolTip="하드포인트 위치를 월드 Transform으로 바꿀 때 우선 기준으로 사용할 컴포넌트 이름입니다. 없으면 차량 Actor Transform을 사용합니다."))
	FName MountParentComponentName = TEXT("SM_Body");

		// [v1.22.0] True이면 VehicleData 기본값 대신 Runtime Equipment Override를 사용합니다.
	UPROPERTY(Transient)
	bool bUseRuntimeEquipmentPresetOverride = false;

	// [v1.22.0] Snapshot이 선택한 EquipmentPresetData입니다. 빈 장착이면 None입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFEquipmentPresetData> RuntimeEquipmentPresetOverride = nullptr;

	// [v1.0.0] 현재 WeaponComp가 활성 장착 프로파일을 사용할 수 있는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="무기 런타임 준비 여부 (bWeaponRuntimeReady)", ToolTip="활성 장착 프로파일과 차량 데이터 참조가 준비되었는지 여부입니다."))
	bool bWeaponRuntimeReady = false;

	// [v1.16.0] 활성 장착 프로파일에서 우선 해석한 EquipmentPresetData입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="활성 EquipmentPresetData (ActiveEquipmentPresetData)", ToolTip="현재 활성 장착 프로파일에서 우선 해석한 EquipmentPresetData입니다."))
	TObjectPtr<UCFEquipmentPresetData> ActiveEquipmentPresetData = nullptr;

		// [v1.19.0] 활성 EquipmentPresetData에서 해석한 TurretMountData입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Launcher", meta=(AllowPrivateAccess="true", DisplayName="활성 TurretMountData (ActiveTurretMountData)", ToolTip="현재 활성 장비 프리셋에서 해석한 TurretMountData입니다. 참조가 바뀌면 Muzzle 순서를 초기화합니다."))
		TObjectPtr<UCFTurretMountData> CachedActiveTurretMountData = nullptr;

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

		// [v1.19.0] 다음 SingleCycle FireOrigin 해결이 검색을 시작할 설정 Muzzle 배열 인덱스입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Launcher", meta=(AllowPrivateAccess="true", DisplayName="다음 Muzzle 인덱스 (NextMuzzleSocketIndex)", ToolTip="다음 발사 원점 계산이 MuzzleSocketNames 배열에서 검색을 시작할 인덱스입니다. 승인 발사 뒤에만 진행합니다."))
	int32 NextMuzzleSocketIndex = 0;

	// [v1.19.0] 마지막 FireOrigin 계산에서 실제로 해결한 Muzzle 이름입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Launcher", meta=(AllowPrivateAccess="true", DisplayName="마지막 해결 Muzzle 이름 (LastResolvedMuzzleSocketName)", ToolTip="마지막 Weapon Aim Solution 계산에서 실제 Pitch 메쉬에 존재해 선택된 Muzzle 소켓 이름입니다."))
	FName LastResolvedMuzzleSocketName = NAME_None;

	// [v1.19.0] 마지막 FireOrigin 계산에서 해결한 설정 Muzzle 배열 인덱스입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Launcher", meta=(AllowPrivateAccess="true", DisplayName="마지막 해결 Muzzle 인덱스 (LastResolvedMuzzleSocketIndex)", ToolTip="마지막 Weapon Aim Solution 계산에서 선택된 MuzzleSocketNames 배열 인덱스입니다."))
	int32 LastResolvedMuzzleSocketIndex = INDEX_NONE;

	// [v1.19.0] 마지막 승인 발사에 실제 사용된 Muzzle 이름입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Launcher", meta=(AllowPrivateAccess="true", DisplayName="마지막 발사 Muzzle 이름 (LastFiredMuzzleSocketName)", ToolTip="마지막 승인 발사 명령에 스냅샷으로 보존된 Muzzle 소켓 이름입니다."))
	FName LastFiredMuzzleSocketName = NAME_None;

	// [v1.19.0] 마지막 승인 발사 뒤 SingleCycle 순서를 진행한 횟수입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon|Launcher", meta=(AllowPrivateAccess="true", DisplayName="Muzzle 순서 진행 횟수 (MuzzleSequenceAdvanceCount)", ToolTip="승인된 발사 결과로 다음 Muzzle 인덱스를 진행한 누적 횟수입니다. 발사 거부는 증가시키지 않습니다."))
	int32 MuzzleSequenceAdvanceCount = 0;

		// [v1.2.0] 마지막으로 승인된 발사 시간입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Weapon", meta=(AllowPrivateAccess="true", DisplayName="마지막 승인 발사 시간 (LastAcceptedFireTimeSeconds)", ToolTip="WeaponComp가 마지막으로 기록한 승인 발사 시간입니다. 아직 없으면 음수입니다."))
	float LastAcceptedFireTimeSeconds = -1.0f;

		// [v1.25.0] single/legacy Runtime에서 실제 무기 내부 Charge 상태를 소유합니다.
	FCFWeaponChargeRuntime ActiveWeaponChargeRuntime;

	// [v1.25.0] 같은 Charge 설정을 가진 다른 WeaponData로 바뀌어도 이전 Charge가 이어지지 않도록 현재 Runtime Source를 추적합니다.
	TWeakObjectPtr<UCFWeaponData> ActiveWeaponChargeSourceData;

	// [v1.23.0] 현재 활성 무기의 실제 Heat 누적·냉각·과열 상태입니다.
	FCFWeaponHeatRuntime ActiveWeaponHeatRuntime;

	// [v1.23.0] 같은 Heat 설정을 가진 다른 WeaponData로 바뀌어도 이전 Heat가 이어지지 않도록 현재 Runtime Source를 추적합니다.
	TWeakObjectPtr<UCFWeaponData> ActiveWeaponHeatSourceData;

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
