// Copyright Epic Games, Inc. All Rights Reserved.

// File: CFVehicleSmoothVisComp.h
// Version: v1.0.0
// Changelog:
// - v1.0.0: CFNetSmooth 수신 Transform을 실제 차량 Actor가 아닌 원격 Visual/Shell 표시 대상에만 적용하는 CarFight용 브리지 컴포넌트 추가.
// Migration:
// - 기존 ACFVehiclePawn에는 자동 부착하지 않는다. 차량 적용 단계에서 별도 Visual/Shell 대상과 CFNetSmooth 컴포넌트를 명시적으로 연결한다.
// Purpose: CarFight 차량 적용 코드가 NetSmoothSync 플러그인 Runtime 코드를 오염시키지 않도록 호스트 프로젝트 쪽 적용 경계를 제공한다.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFVehicleSmoothVisComp.generated.h"

class UCFNetSmoothComp;

UCLASS(ClassGroup=(CarFight), meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleSmoothVisComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// CFNetSmooth Visual/Shell 브리지 컴포넌트의 기본 Tick과 안전 기본값을 설정한다.
	UCFVehicleSmoothVisComp();

	// 수신 Transform을 제공할 CFNetSmooth 컴포넌트를 지정한다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleSmooth", meta=(ToolTip="수신 Transform을 제공할 CF Net Smooth 컴포넌트를 지정합니다. 실제 차량 Actor가 아니라 원격 Visual/Shell 표시 대상에만 사용하세요."))
	void SetNetSmoothSource(UCFNetSmoothComp* InNetSmoothSourceComp);

	// 보간 Transform을 적용할 원격 Visual/Shell 표시 대상 SceneComponent를 지정한다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleSmooth", meta=(ToolTip="보간 Transform을 적용할 원격 Visual/Shell 표시 대상 SceneComponent를 지정합니다. Root, 물리 시뮬레이션 컴포넌트, 실제 VehicleMesh에는 지정하지 마세요."))
	void SetVisualTarget(USceneComponent* InVisualTargetComp);

	// 현재 설정으로 보간 Visual 적용이 가능한지 반환한다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleSmooth", meta=(ToolTip="현재 설정으로 원격 Visual/Shell 보간 적용이 가능한지 반환합니다. 기본적으로 Simulated Proxy에서만 True가 됩니다."))
	bool CanApplySmoothedVisual() const;

	// CFNetSmooth에서 계산한 Transform을 Visual/Shell 표시 대상에 한 번 적용한다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleSmooth", meta=(ToolTip="CF Net Smooth의 현재 보간 Transform을 원격 Visual/Shell 표시 대상에 한 번 적용합니다. 실제 차량 Actor Transform은 변경하지 않습니다."))
	bool ApplySmoothedVisual(float DeltaSeconds);

	// CFNetSmooth Source와 Visual Target이 모두 지정되어 있는지 반환한다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleSmooth", meta=(ToolTip="CF Net Smooth Source와 Visual Target이 모두 지정되어 있는지 반환합니다."))
	bool HasValidVisualBinding() const;

	// 마지막 Visual 적용이 제한 외삽을 사용했는지 반환한다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleSmooth", meta=(ToolTip="마지막 Visual 적용이 제한 외삽 결과를 사용했는지 반환합니다."))
	bool WasLastApplyUsingExtrapolation() const;

	// 보간 Visual Transform을 한 번 이상 적용했는지 반환한다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleSmooth", meta=(ToolTip="보간 Visual Transform을 한 번 이상 적용했는지 반환합니다."))
	bool HasAppliedSmoothedVisual() const;

	// 마지막으로 Visual/Shell 표시 대상에 적용한 Transform을 반환한다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleSmooth", meta=(ToolTip="마지막으로 Visual/Shell 표시 대상에 적용한 Transform을 반환합니다."))
	FTransform GetLastAppliedVisualTransform() const;

protected:
	// 컴포넌트 Tick에서 필요할 때 Visual/Shell 보간을 적용한다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Visual/Shell 보간 적용 기능을 켤지 정한다. 기본값은 안전을 위해 꺼짐이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="차량 Smooth Visual 사용 (bEnableVehicleSmoothVisual)", ToolTip="True이면 CF Net Smooth 수신 Transform을 원격 Visual/Shell 표시 대상에 적용합니다. 실제 차량 Actor에는 적용하지 않습니다."))
	bool bEnableVehicleSmoothVisual = false;

	// Simulated Proxy 차량에서만 Visual/Shell 보간을 적용할지 정한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="Simulated Proxy 전용 적용 (bApplyOnlyToSimulatedProxy)", ToolTip="True이면 원격 Simulated Proxy에서만 Visual/Shell 보간을 적용합니다. 로컬 조작 차량과 서버 권위 Actor에는 적용하지 않습니다."))
	bool bApplyOnlyToSimulatedProxy = true;

	// RootComponent를 Visual Target으로 허용할지 정한다. 기본값은 안전을 위해 꺼짐이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="Root Target 허용 (bAllowRootVisualTarget)", ToolTip="True이면 RootComponent에도 보간 Transform 적용을 허용합니다. 일반 차량 적용에서는 꺼두세요."))
	bool bAllowRootVisualTarget = false;

	// 수신 Transform을 제공하는 CFNetSmooth 컴포넌트 참조다.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="Net Smooth Source (NetSmoothSourceComp)", ToolTip="수신 Transform을 제공하는 CF Net Smooth 컴포넌트입니다."))
	TObjectPtr<UCFNetSmoothComp> NetSmoothSourceComp = nullptr;

	// 보간 Transform을 적용할 원격 Visual/Shell 표시 대상이다.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="Visual Target (VisualTargetComp)", ToolTip="보간 Transform을 적용할 원격 Visual/Shell 표시 대상 SceneComponent입니다."))
	TObjectPtr<USceneComponent> VisualTargetComp = nullptr;

	// 마지막 Visual 적용이 제한 외삽을 사용했는지 저장한다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="마지막 외삽 사용 여부 (bLastApplyUsedExtrapolation)", ToolTip="마지막 Visual 적용이 제한 외삽을 사용했는지 여부입니다."))
	bool bLastApplyUsedExtrapolation = false;

	// 보간 Visual Transform을 한 번 이상 적용했는지 저장한다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="적용 이력 여부 (bHasAppliedSmoothedVisual)", ToolTip="보간 Visual Transform을 한 번 이상 적용했는지 여부입니다."))
	bool bHasAppliedSmoothedVisual = false;

	// 마지막으로 Visual/Shell 표시 대상에 적용한 Transform이다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleSmooth", meta=(AllowPrivateAccess="true", DisplayName="마지막 적용 Transform (LastAppliedVisualTransform)", ToolTip="마지막으로 Visual/Shell 표시 대상에 적용한 월드 Transform입니다."))
	FTransform LastAppliedVisualTransform = FTransform::Identity;
};
