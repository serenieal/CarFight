// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-07-25
// Description: CarFight 전투 FX 에디터 미리보기 Actor
// Scope: PIE 재시작 없이 Niagara 후보의 크기 적용 방식, 위치, 회전과 반복 재생을 에디터 뷰포트에서 조정합니다.
// Changelog:
// - v1.2.0: Component Scale을 무시하는 Niagara를 위한 User Vector Scale 모드와 실제 적용 Scale Debug를 추가.
// - v1.1.0: Editor Viewport 자동 반복 재생, Uniform Scale, Preview 최대 수명 자동 정지와 Override Niagara의 DataAsset 적용을 추가.
// - v1.0.0: 참조 StaticMesh, +X 방향 Arrow, Niagara Preview, 재생·정지·재시작과 DataAsset 값 불러오기·적용 버튼을 추가.
// Migration:
// - 이 Actor는 EditorOnly이며 게임 플레이 전투 판정에 사용하지 않는다.
// - Preview 값을 DataAsset에 적용하면 패키지만 Dirty 처리하므로 사용자가 저장해야 한다.
// - NiagaraUserVector 모드는 Adapted Niagara 내부에서 같은 Vector User Parameter를 크기 계산에 연결해야 실제 시각 크기가 바뀐다.
// - bApplyOverrideNiagaraToDataAsset이 켜져 있고 Override가 지정되면 선택 Niagara도 DataAsset에 함께 적용된다.
// - FAB Niagara 원본은 수정하지 않고 CarFight Adapted 복제본에서 User Parameter를 연결한다.

#pragma once

#include "CoreMinimal.h"
#include "CFCombatFxData.h"
#include "GameFramework/Actor.h"
#include "CFCombatFxPreviewActor.generated.h"

class UArrowComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UStaticMeshComponent;
struct FPropertyChangedEvent;

/** 테스트 맵에 배치해 PIE 없이 Combat FX 후보를 조정하는 Editor 전용 Actor입니다. */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="CarFight Combat FX Preview Actor"))
class CARFIGHT_RE_API ACFCombatFxPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ACFCombatFxPreviewActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|CombatFxPreview|Components")
	TObjectPtr<USceneComponent> PreviewRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|CombatFxPreview|Components", meta=(DisplayName="크기 비교용 참조 메시"))
	TObjectPtr<UStaticMeshComponent> ReferenceMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|CombatFxPreview|Components")
	TObjectPtr<USceneComponent> PreviewOriginComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|CombatFxPreview|Components", meta=(DisplayName="CarFight +X 방출 방향"))
	TObjectPtr<UArrowComponent> DirectionArrowComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|CombatFxPreview|Components")
	TObjectPtr<UNiagaraComponent> PreviewNiagaraComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Source", meta=(DisplayName="미리보기 전투 FX 데이터", ToolTip="값을 불러오거나 최종 Preview 값을 적용할 UCFCombatFxData입니다."))
	TObjectPtr<UCFCombatFxData> PreviewCombatFxData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Source", meta=(DisplayName="Niagara 직접 지정 Override", ToolTip="지정하면 CombatFxData의 NiagaraSystem보다 우선합니다. 후보를 빠르게 비교할 때 사용합니다."))
	TObjectPtr<UNiagaraSystem> PreviewNiagaraSystemOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Source", meta=(DisplayName="Override Niagara를 DataAsset에 적용", ToolTip="Preview 값을 DataAsset에 적용할 때 지정된 Niagara Override도 함께 저장합니다."))
	bool bApplyOverrideNiagaraToDataAsset = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(DisplayName="미리보기 위치 오프셋"))
	FVector PreviewLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(DisplayName="미리보기 회전 오프셋"))
	FRotator PreviewRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(DisplayName="Uniform Scale 사용", ToolTip="켜면 세 축을 PreviewUniformScale 하나로 동시에 조정합니다."))
	bool bUseUniformScale = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(ClampMin="0.001", UIMin="0.001", UIMax="20.0", DisplayName="미리보기 Uniform Scale", ToolTip="세 축에 동일하게 적용할 빠른 크기 조정값입니다."))
	float PreviewUniformScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(DisplayName="미리보기 FX 벡터 스케일", EditCondition="!bUseUniformScale", EditConditionHides, ToolTip="축별 크기 조정이 필요할 때 사용합니다."))
	FVector PreviewFxScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(DisplayName="미리보기 스케일 적용 모드", ToolTip="Component Scale을 무시하는 Niagara는 Niagara User Vector 모드를 선택하고 Adapted Niagara 내부에서 같은 User Parameter를 사용해야 합니다."))
	ECFCombatFxScaleMode PreviewScaleMode = ECFCombatFxScaleMode::ComponentTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Transform", meta=(DisplayName="미리보기 Niagara User Scale Parameter 이름", ToolTip="Niagara User Vector 모드에서 전달할 Vector User Parameter 이름입니다."))
	FName PreviewNiagaraUserScaleParameterName = TEXT("User.CF_FxScale");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Lifetime", meta=(ClampMin="0.0", UIMin="0.0", DisplayName="미리보기 최대 수명 초", ToolTip="Loop FX가 미리보기에서 계속 남지 않도록 자동 정지할 시간이며 DataAsset에도 적용할 수 있습니다. 0이면 자동 정지하지 않습니다."))
	float PreviewMaximumLifetimeSeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Behavior", meta=(DisplayName="프로퍼티 변경 시 자동 재시작"))
	bool bRestartPreviewWhenPropertyChanges = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Behavior", meta=(DisplayName="Construction 시 자동 재생"))
	bool bAutoActivatePreview = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Behavior", meta=(DisplayName="에디터에서 자동 반복 재생", ToolTip="PIE 없이 일회성 FX를 일정 간격으로 계속 재시작합니다."))
	bool bAutoReplayPreview = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Behavior", meta=(ClampMin="0.1", UIMin="0.1", UIMax="10.0", DisplayName="자동 반복 간격 초"))
	float PreviewReplayIntervalSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|CombatFxPreview|Behavior", meta=(DisplayName="최대 수명 후 자동 정지", ToolTip="PreviewMaximumLifetimeSeconds가 지나면 Loop FX를 정지시킵니다. 자동 반복이 켜져 있으면 다음 반복 시 다시 시작합니다."))
	bool bStopPreviewAtMaximumLifetime = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFxPreview|Debug", meta=(DisplayName="마지막 미리보기 요약"))
	FString LastPreviewSummary = TEXT("CombatFxPreview: NotConfigured");

	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|CombatFxPreview", meta=(DisplayName="미리보기 재생"))
	void PreviewOnce();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|CombatFxPreview", meta=(DisplayName="미리보기 재시작"))
	void RestartPreview();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|CombatFxPreview", meta=(DisplayName="미리보기 정지"))
	void StopPreview();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|CombatFxPreview", meta=(DisplayName="DataAsset 값 불러오기"))
	void LoadValuesFromCombatFxData();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|CombatFxPreview", meta=(DisplayName="Preview 값을 DataAsset에 적용"))
	void ApplyPreviewValuesToCombatFxData();

private:
	UNiagaraSystem* ResolvePreviewNiagaraSystem() const;
	FVector ResolvePreviewScale() const;
	void ApplyScaleToPreviewComponent();
	void ApplyPreviewConfiguration(bool bRestartSystem);
	void UpdatePreviewSummary(const TCHAR* StatusText);

	float PreviewReplayElapsedSeconds = 0.0f;
	float PreviewLifetimeElapsedSeconds = 0.0f;
	bool bPreviewCurrentlyPlaying = false;
};
