// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-24
// Description: CarFight 타겟 선택 대표 위치 컴포넌트
// Scope: Blueprint에서 대상별 대표 위치를 조정하고 TargetPoint, Actor Bounds, Actor Location 순서의 안전한 위치 해석을 제공합니다.
// Changelog:
// - v1.0.0: TS-P0-02용 타겟 포인트, 위치 출처, Fallback 해석과 디버그 API를 추가.
// Migration:
// - 기존 대상은 bUseAsTargetPoint=false 기본값으로 기존 Actor Bounds 중심을 유지한다.
// - 차량별 명시 위치가 필요하면 상속된 TargetPoint 컴포넌트를 이동하고 bUseAsTargetPoint를 활성화한다.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CFTargetPointComp.generated.h"

class AActor;

/**
 * 최종 타겟 선택 위치가 어떤 경로에서 해석됐는지 나타냅니다.
 */
UENUM(BlueprintType)
enum class ECFTargetPointSource : uint8
{
	Invalid UMETA(DisplayName="Invalid"),
	TargetPoint UMETA(DisplayName="TargetPoint"),
	ActorBounds UMETA(DisplayName="ActorBounds"),
	ActorLocation UMETA(DisplayName="ActorLocation")
};

/**
 * 후보 평가와 UI 투영에 사용할 최종 타겟 위치 해석 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFTargetPointResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="유효 위치 여부 (bIsValid)", ToolTip="True이면 WorldLocation을 후보 평가와 UI 투영에 사용할 수 있습니다."))
	bool bIsValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="타겟 월드 위치 (WorldLocation)", ToolTip="TargetPoint, Actor Bounds 중심 또는 Actor 위치에서 해석된 최종 월드 위치입니다."))
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="위치 출처 (Source)", ToolTip="최종 월드 위치가 TargetPoint, Actor Bounds 또는 Actor 위치 중 어디에서 해석됐는지 나타냅니다."))
	ECFTargetPointSource Source = ECFTargetPointSource::Invalid;
};

/**
 * Actor 원점과 독립된 선택·표시 대표 위치를 제공하는 SceneComponent입니다.
 * 차량 Blueprint는 상속된 컴포넌트의 상대 위치와 bUseAsTargetPoint만 조정합니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent, DisplayName="CF Target Point"))
class CARFIGHT_RE_API UCFTargetPointComp : public USceneComponent
{
	GENERATED_BODY()

public:
	UCFTargetPointComp();

	// [v1.0.0] 이 컴포넌트를 Actor의 명시적 타겟 위치로 사용할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="타겟 포인트 사용 (bUseAsTargetPoint)", ToolTip="True이면 이 컴포넌트의 월드 위치를 Actor Bounds와 Actor 위치보다 우선 사용합니다. 기존 차량 호환을 위해 기본값은 False입니다."))
		bool bUseAsTargetPoint = false;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="선호 Bounds 자동 정렬", ToolTip="지정 컴포넌트 Bounds 중심 자동 정렬 여부입니다."))
	bool bAutoAlignToPreferredBounds = false;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="선호 Bounds 컴포넌트", ToolTip="자동 정렬 기준 컴포넌트 이름입니다. 차량 기본값은 SM_Body입니다."))
	FName PreferredBoundsComponentName = NAME_None;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="선호 Bounds 로컬 오프셋", ToolTip="Bounds 중심에서 타겟 위치를 미세 조정하는 로컬 오프셋입니다."))
	FVector PreferredBoundsLocalOffset = FVector::ZeroVector;

		UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="선호 Bounds에 타겟 포인트 정렬", ToolTip="지정 컴포넌트의 Bounds 중심에 정렬합니다. 성공하면 True를 반환합니다."))
	bool AlignToPreferredBoundsComponent();

	// [v1.0.0] 이 컴포넌트를 현재 명시적 타겟 포인트로 사용할 수 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="타겟 포인트 사용 가능 여부", ToolTip="bUseAsTargetPoint가 켜져 있고 컴포넌트가 활성 상태인지 반환합니다."))
	bool CanUseAsTargetPoint() const;

	// [v1.0.0] 이 컴포넌트의 현재 월드 위치를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="타겟 포인트 월드 위치 반환", ToolTip="이 컴포넌트의 현재 월드 위치를 반환합니다. 사용 여부와 관계없이 위치 자체를 조회합니다."))
	FVector GetTargetPointWorldLocation() const;

	// [v1.0.0] 소유 Actor의 최종 타겟 위치를 Fallback 규칙으로 해석합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="소유 Actor 타겟 위치 해석", ToolTip="TargetPoint, Actor Bounds 중심, Actor 위치 순서로 소유 Actor의 최종 타겟 위치를 해석합니다."))
	FCFTargetPointResult ResolveOwnerTargetPoint() const;

	// [v1.0.0] 소유 Actor의 현재 위치 해석 결과를 디버그 문자열로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="타겟 포인트 디버그 요약 반환", ToolTip="현재 위치 출처와 월드 위치를 포함한 디버그 요약 문자열을 반환합니다."))
	FString BuildTargetPointDebugSummary() const;

	// [v1.0.0] 소유 Actor와 최종 타겟 위치를 월드 디버그 도형으로 표시합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|TargetPoint", meta=(DisplayName="타겟 포인트 디버그 표시", ToolTip="최종 타겟 위치에 구체를 그리고 Actor 위치에서 타겟 위치까지 선을 표시합니다."))
	void DrawTargetPointDebug(float DurationSeconds = 2.0f, float SphereRadius = 20.0f) const;

	// [v1.0.0] 임의 Actor의 최종 타겟 위치를 공용 Fallback 규칙으로 해석합니다.
	static FCFTargetPointResult ResolveTargetPoint(const AActor* TargetActor);
};
