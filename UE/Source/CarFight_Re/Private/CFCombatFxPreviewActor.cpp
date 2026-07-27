// Copyright (c) CarFight. All Rights Reserved.
// Version: 1.2.0
// Date: 2026-07-25
// Changelog:
// - v1.2.0: Niagara User Vector Scale 적용과 실제 Component Scale Debug를 추가.
// - v1.1.0: Editor Viewport 자동 반복, Uniform Scale, 최대 수명 자동 정지와 Override Niagara DataAsset 적용을 추가.
// - v1.0.0: 기본 EditorOnly Niagara Preview와 DataAsset 값 적용을 구현.

#include "CFCombatFxPreviewActor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

ACFCombatFxPreviewActor::ACFCombatFxPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 0.0f;
	bIsEditorOnlyActor = true;

	PreviewRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot"));
	SetRootComponent(PreviewRootComponent);

	ReferenceMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReferenceMesh"));
	ReferenceMeshComponent->SetupAttachment(PreviewRootComponent);
	ReferenceMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReferenceMeshComponent->SetGenerateOverlapEvents(false);

	PreviewOriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewOrigin"));
	PreviewOriginComponent->SetupAttachment(PreviewRootComponent);

	DirectionArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("EmissionDirection"));
	DirectionArrowComponent->SetupAttachment(PreviewOriginComponent);

	PreviewNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PreviewNiagara"));
	PreviewNiagaraComponent->SetupAttachment(PreviewOriginComponent);
	PreviewNiagaraComponent->SetAutoActivate(false);
	PreviewNiagaraComponent->SetAutoDestroy(false);
	PreviewNiagaraComponent->SetAbsolute(false, false, false);
}

void ACFCombatFxPreviewActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UWorld* World = GetWorld();
	if (!World || World->IsGameWorld() || !ResolvePreviewNiagaraSystem())
	{
		return;
	}

	const float SafeDeltaSeconds = FMath::Max(0.0f, DeltaSeconds);

	if (bPreviewCurrentlyPlaying)
	{
		PreviewLifetimeElapsedSeconds += SafeDeltaSeconds;

		if (bStopPreviewAtMaximumLifetime && PreviewMaximumLifetimeSeconds > 0.0f && PreviewLifetimeElapsedSeconds >= PreviewMaximumLifetimeSeconds)
		{
			if (PreviewNiagaraComponent)
			{
				PreviewNiagaraComponent->DeactivateImmediate();
				PreviewNiagaraComponent->ResetSystem();
			}

			bPreviewCurrentlyPlaying = false;
			PreviewLifetimeElapsedSeconds = 0.0f;
			UpdatePreviewSummary(TEXT("MaximumLifetimeStopped"));
		}
	}

	if (!bAutoReplayPreview)
	{
		return;
	}

	PreviewReplayElapsedSeconds += SafeDeltaSeconds;
	const float SafeReplayIntervalSeconds = FMath::Max(0.1f, PreviewReplayIntervalSeconds);
	if (PreviewReplayElapsedSeconds >= SafeReplayIntervalSeconds)
	{
		RestartPreview();
	}
}

void ACFCombatFxPreviewActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPreviewConfiguration(bAutoActivatePreview);
}

bool ACFCombatFxPreviewActor::ShouldTickIfViewportsOnly() const
{
	return true;
}

#if WITH_EDITOR
void ACFCombatFxPreviewActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyPreviewConfiguration(bRestartPreviewWhenPropertyChanges);
}
#endif

void ACFCombatFxPreviewActor::PreviewOnce()
{
	ApplyPreviewConfiguration(true);
}

void ACFCombatFxPreviewActor::RestartPreview()
{
	ApplyPreviewConfiguration(true);
}

void ACFCombatFxPreviewActor::StopPreview()
{
	if (PreviewNiagaraComponent)
	{
		PreviewNiagaraComponent->DeactivateImmediate();
		PreviewNiagaraComponent->ResetSystem();
	}

	bPreviewCurrentlyPlaying = false;
	PreviewReplayElapsedSeconds = 0.0f;
	PreviewLifetimeElapsedSeconds = 0.0f;
	UpdatePreviewSummary(TEXT("Stopped"));
}

void ACFCombatFxPreviewActor::LoadValuesFromCombatFxData()
{
	if (!PreviewCombatFxData)
	{
		UpdatePreviewSummary(TEXT("LoadFailed_NoCombatFxData"));
		return;
	}

	PreviewFxScale = PreviewCombatFxData->FxScale;
	PreviewScaleMode = PreviewCombatFxData->FxScaleMode;
	PreviewNiagaraUserScaleParameterName = PreviewCombatFxData->NiagaraUserScaleParameterName;
	PreviewRotationOffset = PreviewCombatFxData->RotationOffset;
	PreviewMaximumLifetimeSeconds = PreviewCombatFxData->MaximumLifetimeSeconds;

	const bool bScaleIsUniform = FMath::IsNearlyEqual(PreviewFxScale.X, PreviewFxScale.Y) && FMath::IsNearlyEqual(PreviewFxScale.X, PreviewFxScale.Z);
	bUseUniformScale = bScaleIsUniform;
	if (bScaleIsUniform)
	{
		PreviewUniformScale = PreviewFxScale.X;
	}

	ApplyPreviewConfiguration(true);
}

void ACFCombatFxPreviewActor::ApplyPreviewValuesToCombatFxData()
{
	if (!PreviewCombatFxData)
	{
		UpdatePreviewSummary(TEXT("ApplyFailed_NoCombatFxData"));
		return;
	}

#if WITH_EDITOR
	PreviewCombatFxData->Modify();

	if (bApplyOverrideNiagaraToDataAsset && PreviewNiagaraSystemOverride)
	{
		PreviewCombatFxData->NiagaraSystem = PreviewNiagaraSystemOverride;
	}

	PreviewCombatFxData->FxScale = ResolvePreviewScale();
	PreviewCombatFxData->FxScaleMode = PreviewScaleMode;
	PreviewCombatFxData->NiagaraUserScaleParameterName = PreviewNiagaraUserScaleParameterName;
	PreviewCombatFxData->RotationOffset = PreviewRotationOffset;
	PreviewCombatFxData->MaximumLifetimeSeconds = FMath::Max(0.0f, PreviewMaximumLifetimeSeconds);
	PreviewCombatFxData->PostEditChange();
	PreviewCombatFxData->MarkPackageDirty();
	UpdatePreviewSummary(TEXT("AppliedToDataAsset_SaveRequired"));
#else
	UpdatePreviewSummary(TEXT("ApplyFailed_EditorOnly"));
#endif
}

UNiagaraSystem* ACFCombatFxPreviewActor::ResolvePreviewNiagaraSystem() const
{
	if (PreviewNiagaraSystemOverride)
	{
		return PreviewNiagaraSystemOverride.Get();
	}

	return PreviewCombatFxData ? PreviewCombatFxData->NiagaraSystem.Get() : nullptr;
}

FVector ACFCombatFxPreviewActor::ResolvePreviewScale() const
{
	if (bUseUniformScale)
	{
		const float SafeUniformScale = FMath::Max(0.001f, PreviewUniformScale);
		return FVector(SafeUniformScale);
	}

	FVector SafePreviewScale = PreviewFxScale;
	if (SafePreviewScale.ContainsNaN())
	{
		SafePreviewScale = FVector::OneVector;
	}

	SafePreviewScale.X = FMath::Max(0.001f, SafePreviewScale.X);
	SafePreviewScale.Y = FMath::Max(0.001f, SafePreviewScale.Y);
	SafePreviewScale.Z = FMath::Max(0.001f, SafePreviewScale.Z);
	return SafePreviewScale;
}

void ACFCombatFxPreviewActor::ApplyScaleToPreviewComponent()
{
	if (!PreviewNiagaraComponent)
	{
		return;
	}

	const FVector EffectiveScale = ResolvePreviewScale();
	const bool bUseNiagaraUserScale = PreviewScaleMode == ECFCombatFxScaleMode::NiagaraUserVector && !PreviewNiagaraUserScaleParameterName.IsNone();

	PreviewNiagaraComponent->SetAbsolute(false, false, false);
	if (bUseNiagaraUserScale)
	{
		PreviewNiagaraComponent->SetRelativeScale3D(FVector::OneVector);
		PreviewNiagaraComponent->SetVariableVec3(PreviewNiagaraUserScaleParameterName, EffectiveScale);
	}
	else
	{
		PreviewNiagaraComponent->SetRelativeScale3D(EffectiveScale);
	}
}

void ACFCombatFxPreviewActor::ApplyPreviewConfiguration(bool bRestartSystem)
{
	if (!PreviewOriginComponent || !PreviewNiagaraComponent)
	{
		LastPreviewSummary = TEXT("CombatFxPreview: MissingRequiredComponent");
		return;
	}

	PreviewOriginComponent->SetRelativeLocation(PreviewLocationOffset);
	PreviewOriginComponent->SetRelativeRotation(PreviewRotationOffset);

	UNiagaraSystem* SelectedNiagaraSystem = ResolvePreviewNiagaraSystem();
	PreviewNiagaraComponent->SetAsset(SelectedNiagaraSystem);

	if (!SelectedNiagaraSystem)
	{
		PreviewNiagaraComponent->DeactivateImmediate();
		bPreviewCurrentlyPlaying = false;
		PreviewReplayElapsedSeconds = 0.0f;
		PreviewLifetimeElapsedSeconds = 0.0f;
		UpdatePreviewSummary(TEXT("MissingNiagaraSystem"));
		return;
	}

	if (bRestartSystem)
	{
		PreviewNiagaraComponent->DeactivateImmediate();
		PreviewNiagaraComponent->ReinitializeSystem();
		ApplyScaleToPreviewComponent();
		PreviewNiagaraComponent->Activate(true);
		bPreviewCurrentlyPlaying = true;
		PreviewReplayElapsedSeconds = 0.0f;
		PreviewLifetimeElapsedSeconds = 0.0f;
		UpdatePreviewSummary(TEXT("Playing"));
	}
	else
	{
		ApplyScaleToPreviewComponent();
		UpdatePreviewSummary(TEXT("Configured"));
	}
}

void ACFCombatFxPreviewActor::UpdatePreviewSummary(const TCHAR* StatusText)
{
	const UNiagaraSystem* SelectedNiagaraSystem = ResolvePreviewNiagaraSystem();
	const FVector EffectiveScale = ResolvePreviewScale();
	const FVector ActualComponentScale = PreviewNiagaraComponent ? PreviewNiagaraComponent->GetRelativeScale3D() : FVector::ZeroVector;
	const TCHAR* ScaleModeText = PreviewScaleMode == ECFCombatFxScaleMode::NiagaraUserVector ? TEXT("NiagaraUserVector") : TEXT("ComponentTransform");

	LastPreviewSummary = FString::Printf(
		TEXT("CombatFxPreview: Status=%s, Data=%s, Niagara=%s, Location=(%.1f, %.1f, %.1f), Rotation=(P=%.1f, Y=%.1f, R=%.1f), EffectiveScale=(%.3f, %.3f, %.3f), ComponentScale=(%.3f, %.3f, %.3f), ScaleMode=%s, UserParameter=%s, MaximumLifetime=%.2fs, AutoReplay=%s, ReplayInterval=%.2fs"),
		StatusText,
		PreviewCombatFxData ? *PreviewCombatFxData->GetName() : TEXT("None"),
		SelectedNiagaraSystem ? *SelectedNiagaraSystem->GetName() : TEXT("None"),
		PreviewLocationOffset.X,
		PreviewLocationOffset.Y,
		PreviewLocationOffset.Z,
		PreviewRotationOffset.Pitch,
		PreviewRotationOffset.Yaw,
		PreviewRotationOffset.Roll,
		EffectiveScale.X,
		EffectiveScale.Y,
		EffectiveScale.Z,
		ActualComponentScale.X,
		ActualComponentScale.Y,
		ActualComponentScale.Z,
		ScaleModeText,
		*PreviewNiagaraUserScaleParameterName.ToString(),
		PreviewMaximumLifetimeSeconds,
		bAutoReplayPreview ? TEXT("True") : TEXT("False"),
		PreviewReplayIntervalSeconds);
}
