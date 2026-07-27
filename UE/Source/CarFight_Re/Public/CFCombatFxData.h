// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-07-25
// Description: CarFight 전투 FX DataAsset
// Scope: FAB Niagara 시스템, 크기 적용 방식, 회전 보정과 Loop 오연결 안전 종료 시간을 데이터로 관리합니다.
// Changelog:
// - v1.2.0: 컴포넌트 Transform Scale을 무시하는 Niagara를 위해 User Vector Scale 적용 모드를 추가.
// - v1.1.0: Loop Niagara의 영구 잔류를 막는 MaximumLifetimeSeconds 안전 퓨즈를 추가.
// - v1.0.0: 일회성 Niagara 시스템, 스케일, 회전 오프셋과 설정 요약을 추가.
// Migration:
// - 기존 CombatFxData는 ComponentTransform 기본 모드로 기존 동작을 유지한다.
// - NiagaraUserVector 모드는 Niagara 안에 같은 이름의 Vector User Parameter가 있고 내부 크기 계산에서 실제로 사용될 때만 시각 크기가 바뀐다.
// - FAB 원본 Niagara 자산은 직접 수정하지 않고 CarFight Adapted 복제본에서 User Parameter를 연결한다.
// - MaximumLifetimeSeconds는 Loop 자산을 정상 후보로 허용하는 기능이 아니라 영구 잔류 방지 안전장치다.
// - NiagaraSystem이 비어 있어도 발사, 피격, 피해와 파괴 판정은 그대로 유지한다.
// - 게임 사운드 필드와 런타임은 추가하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFCombatFxData.generated.h"

class UNiagaraSystem;

/** CombatFxData의 FxScale을 Niagara에 어떤 방식으로 전달할지 결정합니다. */
UENUM(BlueprintType)
enum class ECFCombatFxScaleMode : uint8
{
	ComponentTransform UMETA(DisplayName="컴포넌트 Transform Scale", ToolTip="NiagaraComponent의 Transform Scale로 FxScale을 적용합니다. Local Space 또는 Owner Scale을 따르는 일반 Niagara에 사용합니다."),
	NiagaraUserVector UMETA(DisplayName="Niagara User Vector Parameter", ToolTip="컴포넌트 Scale은 1로 유지하고 지정한 Niagara Vector User Parameter에 FxScale을 전달합니다. 월드 단위 SpriteSize를 직접 쓰는 Niagara Adapted 자산에 사용합니다.")
};

/** 발사, Impact 또는 파괴에 사용할 단일 Niagara FX 설정입니다. */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFCombatFxData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Identity", meta=(DisplayName="전투 FX ID (CombatFxId)", ToolTip="디버그와 데이터 구분에 사용할 안정적인 이름입니다."))
	FName CombatFxId = TEXT("ProtoCombatFx");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Visual", meta=(DisplayName="Niagara 시스템 (NiagaraSystem)", ToolTip="재생할 Niagara System입니다. 비어 있으면 FX만 건너뜁니다."))
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Transform", meta=(DisplayName="FX 스케일 (FxScale)", ToolTip="ComponentTransform 모드에서는 NiagaraComponent Transform Scale로, NiagaraUserVector 모드에서는 지정 User Vector Parameter 값으로 전달됩니다."))
	FVector FxScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Transform", meta=(DisplayName="FX 스케일 적용 모드 (FxScaleMode)", ToolTip="Niagara 자산이 Component Scale을 무시하면 NiagaraUserVector를 사용하고 Adapted Niagara에서 User Parameter를 내부 크기 계산에 연결합니다."))
	ECFCombatFxScaleMode FxScaleMode = ECFCombatFxScaleMode::ComponentTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Transform", meta=(DisplayName="Niagara User Scale Parameter 이름", ToolTip="NiagaraUserVector 모드에서 FxScale을 전달할 Vector User Parameter 이름입니다. Adapted Niagara에도 같은 이름과 Vector 타입으로 만들어야 합니다."))
	FName NiagaraUserScaleParameterName = TEXT("User.CF_FxScale");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Transform", meta=(DisplayName="FX 회전 오프셋 (RotationOffset)", ToolTip="CarFight 기준 방향과 FAB Niagara 방출 축이 다를 때 적용할 회전 보정입니다."))
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|CombatFx|Lifetime", meta=(ClampMin="0.0", UIMin="0.0", DisplayName="최대 수명 초 (MaximumLifetimeSeconds)", ToolTip="Loop Niagara가 실수로 연결돼도 영구 잔류하지 않도록 이 시간이 지나면 강제 제거합니다. 0이면 안전 종료를 사용하지 않습니다."))
	float MaximumLifetimeSeconds = 5.0f;

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx", meta=(DisplayName="전투 FX 설정 여부 (Is Combat FX Configured)"))
	bool IsCombatFxConfigured() const;

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx", meta=(DisplayName="전투 FX 요약 생성 (Build Combat FX Summary)"))
	FString BuildCombatFxSummary() const;
};
